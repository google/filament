//===- subzero/src/IceInstARM32.cpp - ARM32 instruction implementation ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the InstARM32 and OperandARM32 classes, primarily the
/// constructors and the dump()/emit() methods.
///
//===----------------------------------------------------------------------===//

#include "IceInstARM32.h"

#include "IceAssemblerARM32.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLoweringARM32.h"

namespace Ice {
namespace ARM32 {

namespace {

using Register = RegARM32::AllRegisters;

// maximum number of registers allowed in vpush/vpop.
static constexpr SizeT VpushVpopMaxConsecRegs = 16;

const struct TypeARM32Attributes_ {
  const char *WidthString;     // b, h, <blank>, or d
  const char *FpWidthString;   // i8, i16, i32, f32, f64
  const char *SVecWidthString; // s8, s16, s32, f32
  const char *UVecWidthString; // u8, u16, u32, f32
  int8_t SExtAddrOffsetBits;
  int8_t ZExtAddrOffsetBits;
} TypeARM32Attributes[] = {
#define X(tag, elementty, int_width, fp_width, uvec_width, svec_width, sbits,  \
          ubits, rraddr, shaddr)                                               \
  {int_width, fp_width, svec_width, uvec_width, sbits, ubits},
    ICETYPEARM32_TABLE
#undef X
};

const struct InstARM32ShiftAttributes_ {
  const char *EmitString;
} InstARM32ShiftAttributes[] = {
#define X(tag, emit) {emit},
    ICEINSTARM32SHIFT_TABLE
#undef X
};

const struct InstARM32CondAttributes_ {
  CondARM32::Cond Opposite;
  const char *EmitString;
} InstARM32CondAttributes[] = {
#define X(tag, encode, opp, emit) {CondARM32::opp, emit},
    ICEINSTARM32COND_TABLE
#undef X
};

size_t getVecElmtBitsize(Type Ty) {
  return typeWidthInBytes(typeElementType(Ty)) * CHAR_BIT;
}

const char *getWidthString(Type Ty) {
  return TypeARM32Attributes[Ty].WidthString;
}

const char *getFpWidthString(Type Ty) {
  return TypeARM32Attributes[Ty].FpWidthString;
}

const char *getSVecWidthString(Type Ty) {
  return TypeARM32Attributes[Ty].SVecWidthString;
}

const char *getUVecWidthString(Type Ty) {
  return TypeARM32Attributes[Ty].UVecWidthString;
}

const char *getVWidthString(Type Ty, InstARM32::FPSign SignType) {
  switch (SignType) {
  case InstARM32::FS_None:
    return getFpWidthString(Ty);
  case InstARM32::FS_Signed:
    return getSVecWidthString(Ty);
  case InstARM32::FS_Unsigned:
    return getUVecWidthString(Ty);
  }
  llvm_unreachable("Invalid Sign Type.");
  return getFpWidthString(Ty);
}

} // end of anonymous namespace

const char *InstARM32Pred::predString(CondARM32::Cond Pred) {
  return InstARM32CondAttributes[Pred].EmitString;
}

void InstARM32Pred::dumpOpcodePred(Ostream &Str, const char *Opcode,
                                   Type Ty) const {
  Str << Opcode << getPredicate() << "." << Ty;
}

CondARM32::Cond InstARM32::getOppositeCondition(CondARM32::Cond Cond) {
  return InstARM32CondAttributes[Cond].Opposite;
}

void InstARM32::startNextInst(const Cfg *Func) const {
  if (auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>())
    Asm->incEmitTextSize(InstSize);
}

void InstARM32::emitUsingTextFixup(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  GlobalContext *Ctx = Func->getContext();
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (getFlags().getDisableHybridAssembly() &&
      getFlags().getSkipUnimplemented()) {
    Asm->trap();
    Asm->resetNeedsTextFixup();
    return;
  }
  std::string Buffer;
  llvm::raw_string_ostream StrBuf(Buffer);
  OstreamLocker L(Ctx);
  Ostream &OldStr = Ctx->getStrEmit();
  Ctx->setStrEmit(StrBuf);
  // Start counting instructions here, so that emit() methods don't
  // need to call this for the first instruction.
  Asm->resetEmitTextSize();
  Asm->incEmitTextSize(InstSize);
  emit(Func);
  Ctx->setStrEmit(OldStr);
  if (getFlags().getDisableHybridAssembly()) {
    if (getFlags().getSkipUnimplemented()) {
      Asm->trap();
    } else {
      llvm::errs() << "Can't assemble: " << StrBuf.str() << "\n";
      UnimplementedError(getFlags());
    }
    Asm->resetNeedsTextFixup();
    return;
  }
  Asm->emitTextInst(StrBuf.str(), Asm->getEmitTextSize());
}

void InstARM32::emitIAS(const Cfg *Func) const { emitUsingTextFixup(Func); }

void InstARM32Pred::emitUnaryopGPR(const char *Opcode,
                                   const InstARM32Pred *Instr, const Cfg *Func,
                                   bool NeedsWidthSuffix) {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 1);
  Type SrcTy = Instr->getSrc(0)->getType();
  Str << "\t" << Opcode;
  if (NeedsWidthSuffix)
    Str << getWidthString(SrcTy);
  Str << Instr->getPredicate() << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(0)->emit(Func);
}

void InstARM32Pred::emitUnaryopFP(const char *Opcode, FPSign Sign,
                                  const InstARM32Pred *Instr, const Cfg *Func) {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 1);
  Type SrcTy = Instr->getSrc(0)->getType();
  Str << "\t" << Opcode << Instr->getPredicate();
  switch (Sign) {
  case FS_None:
    Str << getFpWidthString(SrcTy);
    break;
  case FS_Signed:
    Str << getSVecWidthString(SrcTy);
    break;
  case FS_Unsigned:
    Str << getUVecWidthString(SrcTy);
    break;
  }
  Str << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(0)->emit(Func);
}

void InstARM32Pred::emitTwoAddr(const char *Opcode, const InstARM32Pred *Instr,
                                const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 2);
  Variable *Dest = Instr->getDest();
  assert(Dest == Instr->getSrc(0));
  Str << "\t" << Opcode << Instr->getPredicate() << "\t";
  Dest->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
}

void InstARM32Pred::emitThreeAddr(const char *Opcode,
                                  const InstARM32Pred *Instr, const Cfg *Func,
                                  bool SetFlags) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 2);
  Str << "\t" << Opcode << (SetFlags ? "s" : "") << Instr->getPredicate()
      << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(0)->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
}

void InstARM32::emitThreeAddrFP(const char *Opcode, FPSign SignType,
                                const InstARM32 *Instr, const Cfg *Func,
                                Type OpType) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 2);
  Str << "\t" << Opcode << getVWidthString(OpType, SignType) << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(0)->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
}

void InstARM32::emitFourAddrFP(const char *Opcode, FPSign SignType,
                               const InstARM32 *Instr, const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 3);
  assert(Instr->getSrc(0) == Instr->getDest());
  Str << "\t" << Opcode
      << getVWidthString(Instr->getDest()->getType(), SignType) << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
  Str << ", ";
  Instr->getSrc(2)->emit(Func);
}

void InstARM32Pred::emitFourAddr(const char *Opcode, const InstARM32Pred *Instr,
                                 const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 3);
  Str << "\t" << Opcode << Instr->getPredicate() << "\t";
  Instr->getDest()->emit(Func);
  Str << ", ";
  Instr->getSrc(0)->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
  Str << ", ";
  Instr->getSrc(2)->emit(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32FourAddrGPR<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32FourAddrFP<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32ThreeAddrFP<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32ThreeAddrSignAwareFP<K>::emitIAS(const Cfg *Func) const {
  InstARM32::emitUsingTextFixup(Func);
}

template <> void InstARM32Mla::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 3);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->mla(getDest(), getSrc(0), getSrc(1), getSrc(2), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Mls::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 3);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->mls(getDest(), getSrc(0), getSrc(1), getSrc(2), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Pred::emitCmpLike(const char *Opcode, const InstARM32Pred *Instr,
                                const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Instr->getSrcSize() == 2);
  Str << "\t" << Opcode << Instr->getPredicate() << "\t";
  Instr->getSrc(0)->emit(Func);
  Str << ", ";
  Instr->getSrc(1)->emit(Func);
}

OperandARM32Mem::OperandARM32Mem(Cfg * /* Func */, Type Ty, Variable *Base,
                                 ConstantInteger32 *ImmOffset, AddrMode Mode)
    : OperandARM32(kMem, Ty), Base(Base), ImmOffset(ImmOffset), Index(nullptr),
      ShiftOp(kNoShift), ShiftAmt(0), Mode(Mode) {
  // The Neg modes are only needed for Reg +/- Reg.
  assert(!isNegAddrMode());
  NumVars = 1;
  Vars = &this->Base;
}

OperandARM32Mem::OperandARM32Mem(Cfg *Func, Type Ty, Variable *Base,
                                 Variable *Index, ShiftKind ShiftOp,
                                 uint16_t ShiftAmt, AddrMode Mode)
    : OperandARM32(kMem, Ty), Base(Base), ImmOffset(0), Index(Index),
      ShiftOp(ShiftOp), ShiftAmt(ShiftAmt), Mode(Mode) {
  if (Index->isRematerializable()) {
    llvm::report_fatal_error("Rematerializable Index Register is not allowed.");
  }
  NumVars = 2;
  Vars = Func->allocateArrayOf<Variable *>(2);
  Vars[0] = Base;
  Vars[1] = Index;
}

OperandARM32ShAmtImm::OperandARM32ShAmtImm(ConstantInteger32 *SA)
    : OperandARM32(kShAmtImm, IceType_i8), ShAmt(SA) {}

bool OperandARM32Mem::canHoldOffset(Type Ty, bool SignExt, int32_t Offset) {
  int32_t Bits = SignExt ? TypeARM32Attributes[Ty].SExtAddrOffsetBits
                         : TypeARM32Attributes[Ty].ZExtAddrOffsetBits;
  if (Bits == 0)
    return Offset == 0;
  // Note that encodings for offsets are sign-magnitude for ARM, so we check
  // with IsAbsoluteUint().
  // Scalar fp, and vector types require an offset that is aligned to a multiple
  // of 4.
  if (isScalarFloatingType(Ty) || isVectorType(Ty))
    return Utils::IsAligned(Offset, 4) && Utils::IsAbsoluteUint(Bits, Offset);
  return Utils::IsAbsoluteUint(Bits, Offset);
}

OperandARM32FlexImm::OperandARM32FlexImm(Cfg * /* Func */, Type Ty,
                                         uint32_t Imm, uint32_t RotateAmt)
    : OperandARM32Flex(kFlexImm, Ty), Imm(Imm), RotateAmt(RotateAmt) {
  NumVars = 0;
  Vars = nullptr;
}

bool OperandARM32FlexImm::canHoldImm(uint32_t Immediate, uint32_t *RotateAmt,
                                     uint32_t *Immed_8) {
  // Avoid the more expensive test for frequent small immediate values.
  if (Immediate <= 0xFF) {
    *RotateAmt = 0;
    *Immed_8 = Immediate;
    return true;
  }
  // Note that immediate must be unsigned for the test to work correctly.
  for (int Rot = 1; Rot < 16; Rot++) {
    uint32_t Imm8 = Utils::rotateLeft32(Immediate, 2 * Rot);
    if (Imm8 <= 0xFF) {
      *RotateAmt = Rot;
      *Immed_8 = Imm8;
      return true;
    }
  }
  return false;
}

OperandARM32FlexFpImm::OperandARM32FlexFpImm(Cfg * /*Func*/, Type Ty,
                                             uint32_t ModifiedImm)
    : OperandARM32Flex(kFlexFpImm, Ty), ModifiedImm(ModifiedImm) {}

bool OperandARM32FlexFpImm::canHoldImm(const Operand *C,
                                       uint32_t *ModifiedImm) {
  switch (C->getType()) {
  default:
    llvm::report_fatal_error("Unhandled fp constant type.");
  case IceType_f32: {
    // We violate llvm naming conventions a bit here so that the constants are
    // named after the bit fields they represent. See "A7.5.1 Operation of
    // modified immediate constants, Floating-point" in the ARM ARM.
    static constexpr uint32_t a = 0x80000000u;
    static constexpr uint32_t B = 0x40000000;
    static constexpr uint32_t bbbbb = 0x3E000000;
    static constexpr uint32_t cdefgh = 0x01F80000;
    static constexpr uint32_t AllowedBits = a | B | bbbbb | cdefgh;
    static_assert(AllowedBits == 0xFFF80000u,
                  "Invalid mask for f32 modified immediates.");
    const float F32 = llvm::cast<const ConstantFloat>(C)->getValue();
    const uint32_t I32 = Utils::bitCopy<uint32_t>(F32);
    if (I32 & ~AllowedBits) {
      // constant has disallowed bits.
      return false;
    }

    if ((I32 & bbbbb) != bbbbb && (I32 & bbbbb)) {
      // not all bbbbb bits are 0 or 1.
      return false;
    }

    if (((I32 & B) != 0) == ((I32 & bbbbb) != 0)) {
      // B ^ b = 0;
      return false;
    }

    *ModifiedImm = ((I32 & a) ? 0x80 : 0x00) | ((I32 & bbbbb) ? 0x40 : 0x00) |
                   ((I32 & cdefgh) >> 19);
    return true;
  }
  case IceType_f64: {
    static constexpr uint32_t a = 0x80000000u;
    static constexpr uint32_t B = 0x40000000;
    static constexpr uint32_t bbbbbbbb = 0x3FC00000;
    static constexpr uint32_t cdefgh = 0x003F0000;
    static constexpr uint32_t AllowedBits = a | B | bbbbbbbb | cdefgh;
    static_assert(AllowedBits == 0xFFFF0000u,
                  "Invalid mask for f64 modified immediates.");
    const double F64 = llvm::cast<const ConstantDouble>(C)->getValue();
    const uint64_t I64 = Utils::bitCopy<uint64_t>(F64);
    if (I64 & 0xFFFFFFFFu) {
      // constant has disallowed bits.
      return false;
    }
    const uint32_t I32 = I64 >> 32;

    if (I32 & ~AllowedBits) {
      // constant has disallowed bits.
      return false;
    }

    if ((I32 & bbbbbbbb) != bbbbbbbb && (I32 & bbbbbbbb)) {
      // not all bbbbb bits are 0 or 1.
      return false;
    }

    if (((I32 & B) != 0) == ((I32 & bbbbbbbb) != 0)) {
      // B ^ b = 0;
      return false;
    }

    *ModifiedImm = ((I32 & a) ? 0x80 : 0x00) |
                   ((I32 & bbbbbbbb) ? 0x40 : 0x00) | ((I32 & cdefgh) >> 16);
    return true;
  }
  }
}

OperandARM32FlexFpZero::OperandARM32FlexFpZero(Cfg * /*Func*/, Type Ty)
    : OperandARM32Flex(kFlexFpZero, Ty) {}

OperandARM32FlexReg::OperandARM32FlexReg(Cfg *Func, Type Ty, Variable *Reg,
                                         ShiftKind ShiftOp, Operand *ShiftAmt)
    : OperandARM32Flex(kFlexReg, Ty), Reg(Reg), ShiftOp(ShiftOp),
      ShiftAmt(ShiftAmt) {
  NumVars = 1;
  auto *ShiftVar = llvm::dyn_cast_or_null<Variable>(ShiftAmt);
  if (ShiftVar)
    ++NumVars;
  Vars = Func->allocateArrayOf<Variable *>(NumVars);
  Vars[0] = Reg;
  if (ShiftVar)
    Vars[1] = ShiftVar;
}

InstARM32Br::InstARM32Br(Cfg *Func, const CfgNode *TargetTrue,
                         const CfgNode *TargetFalse,
                         const InstARM32Label *Label, CondARM32::Cond Pred)
    : InstARM32Pred(Func, InstARM32::Br, 0, nullptr, Pred),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse), Label(Label) {}

bool InstARM32Br::optimizeBranch(const CfgNode *NextNode) {
  // If there is no next block, then there can be no fallthrough to optimize.
  if (NextNode == nullptr)
    return false;
  // Intra-block conditional branches can't be optimized.
  if (Label)
    return false;
  // If there is no fallthrough node, such as a non-default case label for a
  // switch instruction, then there is no opportunity to optimize.
  if (getTargetFalse() == nullptr)
    return false;

  // Unconditional branch to the next node can be removed.
  if (isUnconditionalBranch() && getTargetFalse() == NextNode) {
    assert(getTargetTrue() == nullptr);
    setDeleted();
    return true;
  }
  // If the fallthrough is to the next node, set fallthrough to nullptr to
  // indicate.
  if (getTargetFalse() == NextNode) {
    TargetFalse = nullptr;
    return true;
  }
  // If TargetTrue is the next node, and TargetFalse is not nullptr (which was
  // already tested above), then invert the branch condition, swap the targets,
  // and set new fallthrough to nullptr.
  if (getTargetTrue() == NextNode) {
    assert(Predicate != CondARM32::AL);
    setPredicate(getOppositeCondition(getPredicate()));
    TargetTrue = getTargetFalse();
    TargetFalse = nullptr;
    return true;
  }
  return false;
}

bool InstARM32Br::repointEdges(CfgNode *OldNode, CfgNode *NewNode) {
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

template <InstARM32::InstKindARM32 K>
void InstARM32ThreeAddrGPR<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <> void InstARM32Adc::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->adc(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Add::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->add(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32And::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->and_(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Bic::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->bic(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Eor::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->eor(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Asr::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->asr(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Lsl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->lsl(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Lsr::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->lsr(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Orr::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->orr(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Mul::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->mul(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Rsb::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->rsb(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Rsc::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->rsc(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Sbc::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->sbc(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Sdiv::emitIAS(const Cfg *Func) const {
  assert(!SetFlags);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->sdiv(getDest(), getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Sub::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->sub(getDest(), getSrc(0), getSrc(1), SetFlags, getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Udiv::emitIAS(const Cfg *Func) const {
  assert(!SetFlags);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->udiv(getDest(), getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Vadd::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vadd not defined on type " +
                             typeStdString(DestTy));
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vaddqi(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
    break;
  case IceType_v4f32:
    Asm->vaddqf(Dest, getSrc(0), getSrc(1));
    break;
  case IceType_f32:
    Asm->vadds(Dest, getSrc(0), getSrc(1), CondARM32::AL);
    break;
  case IceType_f64:
    Asm->vaddd(Dest, getSrc(0), getSrc(1), CondARM32::AL);
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vand::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vand not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vandq(Dest, getSrc(0), getSrc(1));
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vceq::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type SrcTy = getSrc(0)->getType();
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vceq not defined on type " +
                             typeStdString(SrcTy));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vceqqi(typeElementType(SrcTy), Dest, getSrc(0), getSrc(1));
    break;
  case IceType_v4f32:
    Asm->vceqqs(Dest, getSrc(0), getSrc(1));
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vcge::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type SrcTy = getSrc(0)->getType();
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vcge not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32: {
    const Type ElmtTy = typeElementType(SrcTy);
    assert(Sign != InstARM32::FS_None);
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
      llvm_unreachable("Sign should not be FS_None.");
    case InstARM32::FS_Unsigned:
      Asm->vcugeqi(ElmtTy, Dest, getSrc(0), getSrc(1));
      break;
    case InstARM32::FS_Signed:
      Asm->vcgeqi(ElmtTy, Dest, getSrc(0), getSrc(1));
      break;
    }
  } break;
  case IceType_v4f32:
    Asm->vcgeqs(Dest, getSrc(0), getSrc(1));
    break;
  }
}

template <> void InstARM32Vcgt::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type SrcTy = getSrc(0)->getType();
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vcgt not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32: {
    const Type ElmtTy = typeElementType(SrcTy);
    assert(Sign != InstARM32::FS_None);
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
      llvm_unreachable("Sign should not be FS_None.");
    case InstARM32::FS_Unsigned:
      Asm->vcugtqi(ElmtTy, Dest, getSrc(0), getSrc(1));
      break;
    case InstARM32::FS_Signed:
      Asm->vcgtqi(ElmtTy, Dest, getSrc(0), getSrc(1));
      break;
    }
  } break;
  case IceType_v4f32:
    Asm->vcgtqs(Dest, getSrc(0), getSrc(1));
    break;
  }
}

template <> void InstARM32Vbsl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vbsl not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vbslq(Dest, getSrc(0), getSrc(1));
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vdiv::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    // TODO(kschimpf) Figure if more cases are needed.
    Asm->setNeedsTextFixup();
    break;
  case IceType_f32:
    Asm->vdivs(getDest(), getSrc(0), getSrc(1), CondARM32::AL);
    break;
  case IceType_f64:
    Asm->vdivd(getDest(), getSrc(0), getSrc(1), CondARM32::AL);
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Veor::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  if (isVectorType(Dest->getType())) {
    Asm->veorq(Dest, getSrc(0), getSrc(1));
    assert(!Asm->needsTextFixup());
    return;
  }
  assert(Dest->getType() == IceType_f64);
  Asm->veord(Dest, getSrc(0), getSrc(1));
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vmla::emitIAS(const Cfg *Func) const {
  // Note: Dest == getSrc(0) for four address FP instructions.
  assert(getSrcSize() == 3);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    // TODO(kschimpf) Figure out how vector operations apply.
    emitUsingTextFixup(Func);
    return;
  case IceType_f32:
    Asm->vmlas(getDest(), getSrc(1), getSrc(2), CondARM32::AL);
    assert(!Asm->needsTextFixup());
    return;
  case IceType_f64:
    Asm->vmlad(getDest(), getSrc(1), getSrc(2), CondARM32::AL);
    assert(!Asm->needsTextFixup());
    return;
  }
}

template <> void InstARM32Vmls::emitIAS(const Cfg *Func) const {
  // Note: Dest == getSrc(0) for four address FP instructions.
  assert(getSrcSize() == 3);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    // TODO(kschimpf) Figure out how vector operations apply.
    emitUsingTextFixup(Func);
    return;
  case IceType_f32:
    Asm->vmlss(getDest(), getSrc(1), getSrc(2), CondARM32::AL);
    assert(!Asm->needsTextFixup());
    return;
  case IceType_f64:
    Asm->vmlsd(getDest(), getSrc(1), getSrc(2), CondARM32::AL);
    assert(!Asm->needsTextFixup());
    return;
  }
}

template <> void InstARM32Vmvn::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vmvn not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    Asm->vmvnq(Dest, getSrc(0));
  } break;
  }
}

template <> void InstARM32Vmovl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vmovlq not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    Asm->vmovlq(Dest, getSrc(0), getSrc(1));
  } break;
  }
}

template <> void InstARM32Vmovh::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vmovhq not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    Asm->vmovhq(Dest, getSrc(0), getSrc(1));
  } break;
  }
}

template <> void InstARM32Vmovhl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vmovhlq not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    Asm->vmovhlq(Dest, getSrc(0), getSrc(1));
  } break;
  }
}

template <> void InstARM32Vmovlh::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vmovlhq not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    Asm->vmovlhq(Dest, getSrc(0), getSrc(1));
  } break;
  }
}

template <> void InstARM32Vneg::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type DestTy = Dest->getType();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vneg not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32: {
    const Type ElmtTy = typeElementType(DestTy);
    Asm->vnegqs(ElmtTy, Dest, getSrc(0));
  } break;
  }
}

template <> void InstARM32Vorr::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("Vorr not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vorrq(Dest, getSrc(0), getSrc(1));
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vshl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vshl not defined on type " +
                             typeStdString(Dest->getType()));
  // TODO(jpp): handle i1 vectors in terms of element count instead of element
  // type.
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32: {
    const Type ElmtTy = typeElementType(DestTy);
    assert(Sign != InstARM32::FS_None);
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
    case InstARM32::FS_Unsigned:
      if (const auto *Imm6 = llvm::dyn_cast<ConstantInteger32>(getSrc(1))) {
        Asm->vshlqc(ElmtTy, Dest, getSrc(0), Imm6);
      } else {
        Asm->vshlqu(ElmtTy, Dest, getSrc(0), getSrc(1));
      }
      break;
    case InstARM32::FS_Signed:
      if (const auto *Imm6 = llvm::dyn_cast<ConstantInteger32>(getSrc(1))) {
        Asm->vshlqc(ElmtTy, Dest, getSrc(0), Imm6);
      } else {
        Asm->vshlqi(ElmtTy, Dest, getSrc(0), getSrc(1));
      }
      break;
    }
  } break;
  }
}

template <> void InstARM32Vshr::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vshr not defined on type " +
                             typeStdString(Dest->getType()));
  // TODO(jpp): handle i1 vectors in terms of element count instead of element
  // type.
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32: {
    const Type ElmtTy = typeElementType(DestTy);
    const auto *Imm6 = llvm::cast<ConstantInteger32>(getSrc(1));
    switch (Sign) {
    case InstARM32::FS_Signed:
    case InstARM32::FS_Unsigned:
      Asm->vshrqc(ElmtTy, Dest, getSrc(0), Imm6, Sign);
      break;
    default:
      assert(false && "Vshr requires signedness specification.");
    }
  } break;
  }
}

template <> void InstARM32Vsub::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vsub not defined on type " +
                             typeStdString(DestTy));
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vsubqi(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
    break;
  case IceType_v4f32:
    Asm->vsubqf(Dest, getSrc(0), getSrc(1));
    break;
  case IceType_f32:
    Asm->vsubs(getDest(), getSrc(0), getSrc(1), CondARM32::AL);
    break;
  case IceType_f64:
    Asm->vsubd(getDest(), getSrc(0), getSrc(1), CondARM32::AL);
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vqadd::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vqadd not defined on type " +
                             typeStdString(DestTy));
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
    case InstARM32::FS_Unsigned:
      Asm->vqaddqu(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
      break;
    case InstARM32::FS_Signed:
      Asm->vqaddqi(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
      break;
    }
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vqsub::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vqsub not defined on type " +
                             typeStdString(DestTy));
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
    case InstARM32::FS_Unsigned:
      Asm->vqsubqu(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
      break;
    case InstARM32::FS_Signed:
      Asm->vqsubqi(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
      break;
    }
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vqmovn2::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  const Operand *Src1 = getSrc(1);
  Type SrcTy = Src0->getType();
  Type DestTy = Dest->getType();
  bool Unsigned = true;
  bool Saturating = true;
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vqmovn2 not defined on type " +
                             typeStdString(SrcTy));
  case IceType_v8i16:
  case IceType_v4i32:
    switch (Sign) {
    case InstARM32::FS_None:
      Unsigned = true;
      Saturating = false;
      Asm->vqmovn2(typeElementType(DestTy), Dest, Src0, Src1, Unsigned,
                   Saturating);
      break;
    case InstARM32::FS_Unsigned:
      Unsigned = true;
      Saturating = true;
      Asm->vqmovn2(typeElementType(DestTy), Dest, Src0, Src1, Unsigned,
                   Saturating);
      break;
    case InstARM32::FS_Signed:
      Unsigned = false;
      Saturating = true;
      Asm->vqmovn2(typeElementType(DestTy), Dest, Src0, Src1, Unsigned,
                   Saturating);
      break;
    }
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vmulh::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  Type SrcTy = Src0->getType();
  bool Unsigned = true;
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vmulh not defined on type " +
                             typeStdString(SrcTy));
  case IceType_v8i16:
    switch (Sign) {
    case InstARM32::FS_None: // defaults to unsigned.
    case InstARM32::FS_Unsigned:
      Unsigned = true;
      Asm->vmulh(typeElementType(SrcTy), Dest, getSrc(0), getSrc(1), Unsigned);
      break;
    case InstARM32::FS_Signed:
      Unsigned = false;
      Asm->vmulh(typeElementType(SrcTy), Dest, getSrc(0), getSrc(1), Unsigned);
      break;
    }
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vmlap::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  const Operand *Src1 = getSrc(1);
  Type SrcTy = Src0->getType();
  switch (SrcTy) {
  default:
    llvm::report_fatal_error("Vmlap not defined on type " +
                             typeStdString(SrcTy));
  case IceType_v8i16:
    Asm->vmlap(typeElementType(SrcTy), Dest, Src0, Src1);
    break;
  }
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vzip::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  const Operand *Src1 = getSrc(1);
  Type DestTy = Dest->getType();
  Asm->vzip(typeElementType(DestTy), Dest, Src0, Src1);
  assert(!Asm->needsTextFixup());
}

template <> void InstARM32Vmul::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  const Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Vmul not defined on type " +
                             typeStdString(DestTy));

  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
    Asm->vmulqi(typeElementType(DestTy), Dest, getSrc(0), getSrc(1));
    break;
  case IceType_v4f32:
    Asm->vmulqf(Dest, getSrc(0), getSrc(1));
    break;
  case IceType_f32:
    Asm->vmuls(Dest, getSrc(0), getSrc(1), CondARM32::AL);
    break;
  case IceType_f64:
    Asm->vmuld(Dest, getSrc(0), getSrc(1), CondARM32::AL);
    break;
  }
}

InstARM32Call::InstARM32Call(Cfg *Func, Variable *Dest, Operand *CallTarget)
    : InstARM32(Func, InstARM32::Call, 1, Dest) {
  HasSideEffects = true;
  addSource(CallTarget);
}

InstARM32Label::InstARM32Label(Cfg *Func, TargetARM32 *Target)
    : InstARM32(Func, InstARM32::Label, 0, nullptr),
      Number(Target->makeNextLabelNumber()) {
  if (BuildDefs::dump()) {
    Name = GlobalString::createWithString(
        Func->getContext(),
        ".L" + Func->getFunctionName() + "$local$__" + std::to_string(Number));
  } else {
    Name = GlobalString::createWithoutString(Func->getContext());
  }
}

namespace {
// Requirements for Push/Pop:
//  1) All the Variables have the same type;
//  2) All the variables have registers assigned to them.
void validatePushOrPopRegisterListOrDie(const VarList &RegList) {
  Type PreviousTy = IceType_void;
  for (Variable *Reg : RegList) {
    if (PreviousTy != IceType_void && Reg->getType() != PreviousTy) {
      llvm::report_fatal_error("Type mismatch when popping/pushing "
                               "registers.");
    }

    if (!Reg->hasReg()) {
      llvm::report_fatal_error("Push/pop operand does not have a register "
                               "assigned to it.");
    }

    PreviousTy = Reg->getType();
  }
}
} // end of anonymous namespace

void InstARM32RegisterStackOp::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitUsingForm(Func, Emit_Text);
}

void InstARM32RegisterStackOp::emitIAS(const Cfg *Func) const {
  emitUsingForm(Func, Emit_Binary);
  assert(!Func->getAssembler<ARM32::AssemblerARM32>()->needsTextFixup());
}

void InstARM32RegisterStackOp::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << getDumpOpcode() << " ";
  SizeT NumRegs = getNumStackRegs();
  for (SizeT I = 0; I < NumRegs; ++I) {
    if (I > 0)
      Str << ", ";
    getStackReg(I)->dump(Func);
  }
}

void InstARM32RegisterStackOp::emitGPRsAsText(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t" << getGPROpcode() << "\t{";
  getStackReg(0)->emit(Func);
  const SizeT NumRegs = getNumStackRegs();
  for (SizeT i = 1; i < NumRegs; ++i) {
    Str << ", ";
    getStackReg(i)->emit(Func);
  }
  Str << "}";
}

void InstARM32RegisterStackOp::emitSRegsAsText(const Cfg *Func,
                                               const Variable *BaseReg,
                                               SizeT RegCount) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t" << getSRegOpcode() << "\t{";
  bool IsFirst = true;
  const auto Base = BaseReg->getRegNum();
  for (SizeT i = 0; i < RegCount; ++i) {
    if (IsFirst)
      IsFirst = false;
    else
      Str << ", ";
    Str << RegARM32::getRegName(RegNumT::fixme(Base + i));
  }
  Str << "}";
}

void InstARM32RegisterStackOp::emitSRegsOp(const Cfg *Func, EmitForm Form,
                                           const Variable *BaseReg,
                                           SizeT RegCount,
                                           SizeT InstIndex) const {
  if (Form == Emit_Text && BuildDefs::dump() && InstIndex > 0) {
    startNextInst(Func);
    Func->getContext()->getStrEmit() << "\n";
  }
  emitSRegs(Func, Form, BaseReg, RegCount);
}

namespace {

bool isAssignedConsecutiveRegisters(const Variable *Before,
                                    const Variable *After) {
  assert(Before->hasReg());
  assert(After->hasReg());
  return RegNumT::fixme(Before->getRegNum() + 1) == After->getRegNum();
}

} // end of anonymous namespace

void InstARM32RegisterStackOp::emitUsingForm(const Cfg *Func,
                                             const EmitForm Form) const {
  SizeT NumRegs = getNumStackRegs();
  assert(NumRegs);

  const auto *Reg = llvm::cast<Variable>(getStackReg(0));
  if (isScalarIntegerType(Reg->getType())) {
    // Push/pop GPR registers.
    SizeT IntegerCount = 0;
    ARM32::IValueT GPRegisters = 0;
    const Variable *LastDest = nullptr;
    for (SizeT i = 0; i < NumRegs; ++i) {
      const Variable *Var = getStackReg(i);
      assert(Var->hasReg() && "stack op only applies to registers");
      const RegARM32::GPRRegister Reg =
          RegARM32::getEncodedGPR(Var->getRegNum());
      LastDest = Var;
      GPRegisters |= (1 << Reg);
      ++IntegerCount;
    }
    if (IntegerCount == 1) {
      emitSingleGPR(Func, Form, LastDest);
    } else {
      emitMultipleGPRs(Func, Form, GPRegisters);
    }
    return;
  }

  // Push/pop floating point registers. Divide into a list of instructions,
  // defined on consecutive register ranges. Then generate the corresponding
  // instructions.

  // Typical max number of registers ranges pushed/popd is no more than 5.
  llvm::SmallVector<std::pair<const Variable *, SizeT>, 5> InstData;
  const Variable *BaseReg = nullptr;
  SizeT RegCount = 0;
  for (SizeT i = 0; i < NumRegs; ++i) {
    const Variable *NextReg = getStackReg(i);
    assert(NextReg->hasReg());
    if (BaseReg == nullptr) {
      BaseReg = NextReg;
      RegCount = 1;
    } else if (RegCount < VpushVpopMaxConsecRegs &&
               isAssignedConsecutiveRegisters(Reg, NextReg)) {
      ++RegCount;
    } else {
      InstData.emplace_back(BaseReg, RegCount);
      BaseReg = NextReg;
      RegCount = 1;
    }
    Reg = NextReg;
  }
  if (RegCount) {
    InstData.emplace_back(BaseReg, RegCount);
  }
  SizeT InstCount = 0;
  if (llvm::isa<InstARM32Push>(*this)) {
    for (const auto &Pair : InstData)
      emitSRegsOp(Func, Form, Pair.first, Pair.second, InstCount++);
    return;
  }
  assert(llvm::isa<InstARM32Pop>(*this));
  for (const auto &Pair : reverse_range(InstData))
    emitSRegsOp(Func, Form, Pair.first, Pair.second, InstCount++);
}

InstARM32Pop::InstARM32Pop(Cfg *Func, const VarList &Dests)
    : InstARM32RegisterStackOp(Func, InstARM32::Pop, 0, nullptr), Dests(Dests) {
  // Track modifications to Dests separately via FakeDefs. Also, a pop
  // instruction affects the stack pointer and so it should not be allowed to
  // be automatically dead-code eliminated. This is automatic since we leave
  // the Dest as nullptr.
  validatePushOrPopRegisterListOrDie(Dests);
}

InstARM32Push::InstARM32Push(Cfg *Func, const VarList &Srcs)
    : InstARM32RegisterStackOp(Func, InstARM32::Push, Srcs.size(), nullptr) {
  validatePushOrPopRegisterListOrDie(Srcs);
  for (Variable *Source : Srcs) {
    addSource(Source);
  }
}

InstARM32Ret::InstARM32Ret(Cfg *Func, Variable *LR, Variable *Source)
    : InstARM32(Func, InstARM32::Ret, Source ? 2 : 1, nullptr) {
  addSource(LR);
  if (Source)
    addSource(Source);
}

InstARM32Str::InstARM32Str(Cfg *Func, Variable *Value, OperandARM32Mem *Mem,
                           CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Str, 2, nullptr, Predicate) {
  addSource(Value);
  addSource(Mem);
}

InstARM32Strex::InstARM32Strex(Cfg *Func, Variable *Dest, Variable *Value,
                               OperandARM32Mem *Mem, CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Strex, 2, Dest, Predicate) {
  addSource(Value);
  addSource(Mem);
}

InstARM32Vstr1::InstARM32Vstr1(Cfg *Func, Variable *Value, OperandARM32Mem *Mem,
                               CondARM32::Cond Predicate, SizeT Size)
    : InstARM32Pred(Func, InstARM32::Vstr1, 2, nullptr, Predicate) {
  addSource(Value);
  addSource(Mem);
  this->Size = Size;
}

InstARM32Vdup::InstARM32Vdup(Cfg *Func, Variable *Dest, Variable *Src,
                             IValueT Idx)
    : InstARM32Pred(Func, InstARM32::Vdup, 1, Dest, CondARM32::AL), Idx(Idx) {
  addSource(Src);
}

InstARM32Trap::InstARM32Trap(Cfg *Func)
    : InstARM32(Func, InstARM32::Trap, 0, nullptr) {}

InstARM32Umull::InstARM32Umull(Cfg *Func, Variable *DestLo, Variable *DestHi,
                               Variable *Src0, Variable *Src1,
                               CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Umull, 2, DestLo, Predicate),
      // DestHi is expected to have a FakeDef inserted by the lowering code.
      DestHi(DestHi) {
  addSource(Src0);
  addSource(Src1);
}

InstARM32Vcvt::InstARM32Vcvt(Cfg *Func, Variable *Dest, Variable *Src,
                             VcvtVariant Variant, CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Vcvt, 1, Dest, Predicate),
      Variant(Variant) {
  addSource(Src);
}

InstARM32Mov::InstARM32Mov(Cfg *Func, Variable *Dest, Operand *Src,
                           CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Mov, 2, Dest, Predicate) {
  auto *Dest64 = llvm::dyn_cast<Variable64On32>(Dest);
  auto *Src64 = llvm::dyn_cast<Variable64On32>(Src);

  assert(Dest64 == nullptr || Src64 == nullptr);

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

namespace {

// These next two functions find the D register that maps to the half of the Q
// register that this instruction is accessing.
Register getDRegister(const Variable *Src, uint32_t Index) {
  assert(Src->hasReg());
  const auto SrcReg = Src->getRegNum();

  const RegARM32::RegTableType &SrcEntry = RegARM32::RegTable[SrcReg];
  assert(SrcEntry.IsVec128);

  const uint32_t NumElements = typeNumElements(Src->getType());

  // This code assumes the Aliases list goes Q_n, S_2n, S_2n+1. The asserts in
  // the next two branches help to check that this is still true.
  if (Index < NumElements / 2) {
    // We have a Q register that's made up of two D registers. This assert is
    // to help ensure that we picked the right D register.
    //
    // TODO(jpp): find a way to do this that doesn't rely on ordering of the
    // alias list.
    assert(RegARM32::RegTable[SrcEntry.Aliases[1]].Encoding + 1 ==
           RegARM32::RegTable[SrcEntry.Aliases[2]].Encoding);
    return static_cast<Register>(SrcEntry.Aliases[1]);
  } else {
    // We have a Q register that's made up of two D registers. This assert is
    // to help ensure that we picked the right D register.
    //
    // TODO(jpp): find a way to do this that doesn't rely on ordering of the
    // alias list.
    assert(RegARM32::RegTable[SrcEntry.Aliases[2]].Encoding - 1 ==
           RegARM32::RegTable[SrcEntry.Aliases[1]].Encoding);
    return static_cast<Register>(SrcEntry.Aliases[2]);
  }
}

uint32_t adjustDIndex(Type Ty, uint32_t DIndex) {
  // If Ty is a vector of i1, we may need to adjust DIndex. This is needed
  // because, e.g., the second i1 in a v4i1 is accessed with a
  //
  // vmov.s8 Qd[4], Rn
  switch (Ty) {
  case IceType_v4i1:
    return DIndex * 4;
  case IceType_v8i1:
    return DIndex * 2;
  case IceType_v16i1:
    return DIndex;
  default:
    return DIndex;
  }
}

uint32_t getDIndex(Type Ty, uint32_t NumElements, uint32_t Index) {
  const uint32_t DIndex =
      (Index < NumElements / 2) ? Index : Index - (NumElements / 2);
  return adjustDIndex(Ty, DIndex);
}

// For floating point values, we can insertelement or extractelement by moving
// directly from an S register. This function finds the right one.
Register getSRegister(const Variable *Src, uint32_t Index) {
  assert(Src->hasReg());
  const auto SrcReg = Src->getRegNum();

  // For floating point values, we need to be allocated to Q0 - Q7, so we can
  // directly access the value we want as one of the S registers.
  assert(Src->getType() == IceType_v4f32);
  assert(SrcReg < RegARM32::Reg_q8);

  // This part assumes the register alias list goes q0, d0, d1, s0, s1, s2, s3.
  assert(Index < 4);

  // TODO(jpp): find a way to do this that doesn't rely on ordering of the alias
  // list.
  return static_cast<Register>(RegARM32::RegTable[SrcReg].Aliases[Index + 3]);
}

} // end of anonymous namespace

void InstARM32Extract::emit(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  const Type DestTy = getDest()->getType();

  const auto *Src = llvm::cast<Variable>(getSrc(0));

  if (isIntegerType(DestTy)) {
    Str << "\t"
        << "vmov" << getPredicate();
    const uint32_t BitSize = typeWidthInBytes(DestTy) * CHAR_BIT;
    if (BitSize < 32) {
      Str << ".s" << BitSize;
    } else {
      Str << "." << BitSize;
    }
    Str << "\t";
    getDest()->emit(Func);
    Str << ", ";

    const Type SrcTy = Src->getType();
    const size_t VectorSize = typeNumElements(SrcTy);

    const Register SrcReg = getDRegister(Src, Index);

    Str << RegARM32::RegTable[SrcReg].Name;
    Str << "[" << getDIndex(SrcTy, VectorSize, Index) << "]";
  } else if (isFloatingType(DestTy)) {
    const Register SrcReg = getSRegister(Src, Index);

    Str << "\t"
        << "vmov" << getPredicate() << ".f32"
        << "\t";
    getDest()->emit(Func);
    Str << ", " << RegARM32::RegTable[SrcReg].Name;
  } else {
    assert(false && "Invalid extract type");
  }
}

void InstARM32Extract::emitIAS(const Cfg *Func) const {
  const Operand *Dest = getDest();
  const Type DestTy = Dest->getType();
  const Operand *Src = getSrc(0);
  const Type SrcTy = Src->getType();
  assert(isVectorType(Src->getType()));
  assert(DestTy == typeElementType(Src->getType()));
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (isIntegerType(DestTy)) {
    Asm->vmovrqi(Dest, Src, adjustDIndex(SrcTy, Index), getPredicate());
    assert(!Asm->needsTextFixup());
    return;
  }
  assert(isFloatingType(DestTy));
  Asm->vmovsqi(Dest, Src, Index, getPredicate());
  assert(!Asm->needsTextFixup());
}

namespace {
Type insertionType(Type Ty) {
  assert(isVectorType(Ty));
  switch (Ty) {
  case IceType_v4i1:
    return IceType_v4i32;
  case IceType_v8i1:
    return IceType_v8i16;
  case IceType_v16i1:
    return IceType_v16i8;
  default:
    return Ty;
  }
}
} // end of anonymous namespace

void InstARM32Insert::emit(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  const Variable *Dest = getDest();
  const auto *Src = llvm::cast<Variable>(getSrc(0));
  const Type DestTy = insertionType(getDest()->getType());
  assert(isVectorType(DestTy));

  if (isIntegerType(DestTy)) {
    Str << "\t"
        << "vmov" << getPredicate();
    const size_t BitSize = typeWidthInBytes(typeElementType(DestTy)) * CHAR_BIT;
    Str << "." << BitSize << "\t";

    const size_t VectorSize = typeNumElements(DestTy);
    const Register DestReg = getDRegister(Dest, Index);
    const uint32_t Index =
        getDIndex(insertionType(DestTy), VectorSize, this->Index);
    Str << RegARM32::RegTable[DestReg].Name;
    Str << "[" << Index << "], ";
    Src->emit(Func);
  } else if (isFloatingType(DestTy)) {
    Str << "\t"
        << "vmov" << getPredicate() << ".f32"
        << "\t";
    const Register DestReg = getSRegister(Dest, Index);
    Str << RegARM32::RegTable[DestReg].Name << ", ";
    Src->emit(Func);
  } else {
    assert(false && "Invalid insert type");
  }
}

void InstARM32Insert::emitIAS(const Cfg *Func) const {
  const Variable *Dest = getDest();
  const auto *Src = llvm::cast<Variable>(getSrc(0));
  const Type DestTy = insertionType(Dest->getType());
  const Type SrcTy = typeElementType(DestTy);
  assert(SrcTy == Src->getType() || Src->getType() == IceType_i1);
  assert(isVectorType(DestTy));
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (isIntegerType(SrcTy)) {
    Asm->vmovqir(Dest->asType(Func, DestTy, Dest->getRegNum()),
                 adjustDIndex(DestTy, Index),
                 Src->asType(Func, SrcTy, Src->getRegNum()), getPredicate());
    assert(!Asm->needsTextFixup());
    return;
  }
  assert(isFloatingType(SrcTy));
  Asm->vmovqis(Dest, Index, Src, getPredicate());
  assert(!Asm->needsTextFixup());
}

template <InstARM32::InstKindARM32 K>
void InstARM32CmpLike<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <> void InstARM32Cmn::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->cmn(getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Cmp::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->cmp(getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Tst::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->tst(getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

InstARM32Dmb::InstARM32Dmb(Cfg *Func)
    : InstARM32Pred(Func, InstARM32::Dmb, 0, nullptr, CondARM32::AL) {}

InstARM32Nop::InstARM32Nop(Cfg *Func)
    : InstARM32Pred(Func, InstARM32::Nop, 0, nullptr, CondARM32::AL) {}

InstARM32Vcmp::InstARM32Vcmp(Cfg *Func, Variable *Src0, Operand *Src1,
                             CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Vcmp, 2, nullptr, Predicate) {
  HasSideEffects = true;
  addSource(Src0);
  addSource(Src1);
}

InstARM32Vmrs::InstARM32Vmrs(Cfg *Func, CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Vmrs, 0, nullptr, Predicate) {
  HasSideEffects = true;
}

InstARM32Vabs::InstARM32Vabs(Cfg *Func, Variable *Dest, Variable *Src,
                             CondARM32::Cond Predicate)
    : InstARM32Pred(Func, InstARM32::Vabs, 1, Dest, Predicate) {
  addSource(Src);
}

// ======================== Dump routines ======================== //

void InstARM32::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "[ARM32] ";
  Inst::dump(Func);
}

void InstARM32Mov::emitMultiDestSingleSource(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Variable *DestLo = getDest();
  Variable *DestHi = getDestHi();
  auto *Src = llvm::cast<Variable>(getSrc(0));

  assert(DestHi->hasReg());
  assert(DestLo->hasReg());
  assert(Src->hasReg());

  Str << "\t"
         "vmov"
      << getPredicate() << "\t";
  DestLo->emit(Func);
  Str << ", ";
  DestHi->emit(Func);
  Str << ", ";
  Src->emit(Func);
}

void InstARM32Mov::emitSingleDestMultiSource(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Variable *Dest = getDest();
  auto *SrcLo = llvm::cast<Variable>(getSrc(0));
  auto *SrcHi = llvm::cast<Variable>(getSrc(1));

  assert(SrcHi->hasReg());
  assert(SrcLo->hasReg());
  assert(Dest->hasReg());
  assert(getSrcSize() == 2);

  Str << "\t"
         "vmov"
      << getPredicate() << "\t";
  Dest->emit(Func);
  Str << ", ";
  SrcLo->emit(Func);
  Str << ", ";
  SrcHi->emit(Func);
}

namespace {

bool isVariableWithoutRegister(const Operand *Op) {
  if (const auto *OpV = llvm::dyn_cast<Variable>(Op)) {
    return !OpV->hasReg();
  }
  return false;
}
bool isMemoryAccess(Operand *Op) {
  return isVariableWithoutRegister(Op) || llvm::isa<OperandARM32Mem>(Op);
}

bool isMoveBetweenCoreAndVFPRegisters(Variable *Dest, Operand *Src) {
  const Type DestTy = Dest->getType();
  const Type SrcTy = Src->getType();
  return !isVectorType(DestTy) && !isVectorType(SrcTy) &&
         (isScalarIntegerType(DestTy) == isScalarFloatingType(SrcTy));
}

} // end of anonymous namespace

void InstARM32Mov::emitSingleDestSingleSource(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Variable *Dest = getDest();

  if (!Dest->hasReg()) {
    llvm::report_fatal_error("mov can't store.");
  }

  Operand *Src0 = getSrc(0);
  if (isMemoryAccess(Src0)) {
    llvm::report_fatal_error("mov can't load.");
  }

  Type Ty = Dest->getType();
  const bool IsVector = isVectorType(Ty);
  const bool IsScalarFP = isScalarFloatingType(Ty);
  const bool CoreVFPMove = isMoveBetweenCoreAndVFPRegisters(Dest, Src0);
  const bool IsVMove = (IsVector || IsScalarFP || CoreVFPMove);
  const char *Opcode = IsVMove ? "vmov" : "mov";
  // when vmov{c}'ing, we need to emit a width string. Otherwise, the
  // assembler might be tempted to assume we want a vector vmov{c}, and that
  // is disallowed because ARM.
  const char *WidthString = !CoreVFPMove ? getFpWidthString(Ty) : "";
  CondARM32::Cond Cond = getPredicate();
  if (IsVector)
    assert(CondARM32::isUnconditional(Cond) &&
           "Moves on vectors must be unconditional!");
  Str << "\t" << Opcode;
  if (IsVMove) {
    Str << Cond << WidthString;
  } else {
    Str << WidthString << Cond;
  }
  Str << "\t";
  Dest->emit(Func);
  Str << ", ";
  Src0->emit(Func);
}

void InstARM32Mov::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(!(isMultiDest() && isMultiSource()) && "Invalid vmov type.");
  if (isMultiDest()) {
    emitMultiDestSingleSource(Func);
    return;
  }

  if (isMultiSource()) {
    emitSingleDestMultiSource(Func);
    return;
  }

  emitSingleDestSingleSource(Func);
}

void InstARM32Mov::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  Operand *Src0 = getSrc(0);
  const CondARM32::Cond Cond = getPredicate();
  if (!Dest->hasReg()) {
    llvm::report_fatal_error("mov can't store.");
  }
  if (isMemoryAccess(Src0)) {
    llvm::report_fatal_error("mov can't load.");
  }

  assert(!(isMultiDest() && isMultiSource()) && "Invalid vmov type.");
  if (isMultiDest()) {
    Asm->vmovrrd(Dest, getDestHi(), Src0, Cond);
    return;
  }
  if (isMultiSource()) {
    Asm->vmovdrr(Dest, Src0, getSrc(1), Cond);
    return;
  }

  const Type DestTy = Dest->getType();
  const Type SrcTy = Src0->getType();
  switch (DestTy) {
  default:
    break; // Error
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
    switch (SrcTy) {
    default:
      break; // Error
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32:
    case IceType_i64:
      Asm->mov(Dest, Src0, Cond);
      return;
    case IceType_f32:
      Asm->vmovrs(Dest, Src0, Cond);
      return;
    }
    break; // Error
  case IceType_i64:
    if (isScalarIntegerType(SrcTy)) {
      Asm->mov(Dest, Src0, Cond);
      return;
    }
    if (SrcTy == IceType_f64) {
      if (const auto *Var = llvm::dyn_cast<Variable>(Src0)) {
        Asm->vmovdd(Dest, Var, Cond);
        return;
      }
      if (const auto *FpImm = llvm::dyn_cast<OperandARM32FlexFpImm>(Src0)) {
        Asm->vmovd(Dest, FpImm, Cond);
        return;
      }
    }
    break; // Error
  case IceType_f32:
    switch (SrcTy) {
    default:
      break; // Error
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32:
      return Asm->vmovsr(Dest, Src0, Cond);
    case IceType_f32:
      if (const auto *Var = llvm::dyn_cast<Variable>(Src0)) {
        Asm->vmovss(Dest, Var, Cond);
        return;
      }
      if (const auto *FpImm = llvm::dyn_cast<OperandARM32FlexFpImm>(Src0)) {
        Asm->vmovs(Dest, FpImm, Cond);
        return;
      }
      break; // Error
    }
    break; // Error
  case IceType_f64:
    if (SrcTy == IceType_f64) {
      if (const auto *Var = llvm::dyn_cast<Variable>(Src0)) {
        Asm->vmovdd(Dest, Var, Cond);
        return;
      }
      if (const auto *FpImm = llvm::dyn_cast<OperandARM32FlexFpImm>(Src0)) {
        Asm->vmovd(Dest, FpImm, Cond);
        return;
      }
    }
    break; // Error
  // TODO(jpp): Remove vectors of i1.
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32:
    assert(CondARM32::isUnconditional(Cond) &&
           "Moves on vector must be unconditional!");
    if (isVectorType(SrcTy)) {
      // Mov between different Src and Dest types is used for bitcasting
      // vectors.  We still want to make sure SrcTy is a vector type.
      Asm->vorrq(Dest, Src0, Src0);
      return;
    } else if (const auto *C = llvm::dyn_cast<ConstantInteger32>(Src0)) {
      // Mov with constant argument, allowing the initializing all elements of
      // the vector.
      if (Asm->vmovqc(Dest, C))
        return;
    }
  }
  llvm::report_fatal_error("Mov: don't know how to move " +
                           typeStdString(SrcTy) + " to " +
                           typeStdString(DestTy));
}

void InstARM32Mov::dump(const Cfg *Func) const {
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

  dumpOpcodePred(Str, " = mov", getDest()->getType());
  Str << " ";

  dumpSources(Func);
}

void InstARM32Br::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t"
         "b"
      << getPredicate() << "\t";
  if (Label) {
    Str << Label->getLabelName();
  } else {
    if (isUnconditionalBranch()) {
      Str << getTargetFalse()->getAsmName();
    } else {
      Str << getTargetTrue()->getAsmName();
      if (getTargetFalse()) {
        startNextInst(Func);
        Str << "\n\t"
            << "b"
            << "\t" << getTargetFalse()->getAsmName();
      }
    }
  }
}

void InstARM32Br::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (Label) {
    Asm->b(Asm->getOrCreateLocalLabel(Label->getNumber()), getPredicate());
  } else if (isUnconditionalBranch()) {
    Asm->b(Asm->getOrCreateCfgNodeLabel(getTargetFalse()->getIndex()),
           getPredicate());
  } else {
    Asm->b(Asm->getOrCreateCfgNodeLabel(getTargetTrue()->getIndex()),
           getPredicate());
    if (const CfgNode *False = getTargetFalse())
      Asm->b(Asm->getOrCreateCfgNodeLabel(False->getIndex()), CondARM32::AL);
  }
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Br::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "br ";

  if (getPredicate() == CondARM32::AL) {
    if (Label) {
      Str << "label %" << Label->getLabelName();
    } else {
      Str << "label %" << getTargetFalse()->getName();
    }
    return;
  }

  if (Label) {
    Str << getPredicate() << ", label %" << Label->getLabelName();
  } else {
    Str << getPredicate() << ", label %" << getTargetTrue()->getName();
    if (getTargetFalse()) {
      Str << ", label %" << getTargetFalse()->getName();
    }
  }
}

void InstARM32Call::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  if (llvm::isa<ConstantInteger32>(getCallTarget())) {
    // This shouldn't happen (typically have to copy the full 32-bits to a
    // register and do an indirect jump).
    llvm::report_fatal_error("ARM32Call to ConstantInteger32");
  } else if (const auto *CallTarget =
                 llvm::dyn_cast<ConstantRelocatable>(getCallTarget())) {
    // Calls only have 24-bits, but the linker should insert veneers to extend
    // the range if needed.
    Str << "\t"
           "bl"
           "\t";
    CallTarget->emitWithoutPrefix(Func->getTarget());
  } else {
    Str << "\t"
           "blx"
           "\t";
    getCallTarget()->emit(Func);
  }
}

void InstARM32Call::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (llvm::isa<ConstantInteger32>(getCallTarget())) {
    // This shouldn't happen (typically have to copy the full 32-bits to a
    // register and do an indirect jump).
    llvm::report_fatal_error("ARM32Call to ConstantInteger32");
  } else if (const auto *CallTarget =
                 llvm::dyn_cast<ConstantRelocatable>(getCallTarget())) {
    // Calls only have 24-bits, but the linker should insert veneers to extend
    // the range if needed.
    Asm->bl(CallTarget);
  } else {
    Asm->blx(getCallTarget());
  }
  if (Asm->needsTextFixup())
    return emitUsingTextFixup(Func);
}

void InstARM32Call::dump(const Cfg *Func) const {
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

void InstARM32Label::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  // A label is not really an instruction. Hence, we need to fix the
  // emitted text size.
  if (auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>())
    Asm->decEmitTextSize(InstSize);
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << getLabelName() << ":";
}

void InstARM32Label::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->bindLocalLabel(this, Number);
  if (OffsetReloc != nullptr) {
    Asm->bindRelocOffset(OffsetReloc);
  }
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Label::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << getLabelName() << ":";
}

template <InstARM32::InstKindARM32 K>
void InstARM32LoadBase<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <> void InstARM32Ldr::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Variable *Dest = getDest();
  Type Ty = Dest->getType();
  const bool IsVector = isVectorType(Ty);
  const bool IsScalarFloat = isScalarFloatingType(Ty);
  const char *ActualOpcode =
      IsVector ? "vld1" : (IsScalarFloat ? "vldr" : "ldr");
  const char *WidthString = IsVector ? "" : getWidthString(Ty);
  Str << "\t" << ActualOpcode;
  const bool IsVInst = IsVector || IsScalarFloat;
  if (IsVInst) {
    Str << getPredicate() << WidthString;
  } else {
    Str << WidthString << getPredicate();
  }
  if (IsVector)
    Str << "." << getVecElmtBitsize(Ty);
  Str << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

template <> void InstARM32Vldr1d::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Variable *Dest = getDest();
  Type Ty = Dest->getType();
  const bool IsVector = isVectorType(Ty);
  const bool IsScalarFloat = isScalarFloatingType(Ty);
  const char *ActualOpcode =
      IsVector ? "vld1" : (IsScalarFloat ? "vldr" : "ldr");
  const char *WidthString = IsVector ? "" : getWidthString(Ty);
  Str << "\t" << ActualOpcode;
  const bool IsVInst = IsVector || IsScalarFloat;
  if (IsVInst) {
    Str << getPredicate() << WidthString;
  } else {
    Str << WidthString << getPredicate();
  }
  if (IsVector)
    Str << "." << getVecElmtBitsize(Ty);
  Str << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

template <> void InstARM32Vldr1q::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Variable *Dest = getDest();
  Type Ty = Dest->getType();
  const bool IsVector = isVectorType(Ty);
  const bool IsScalarFloat = isScalarFloatingType(Ty);
  const char *ActualOpcode =
      IsVector ? "vld1" : (IsScalarFloat ? "vldr" : "ldr");
  const char *WidthString = IsVector ? "" : getWidthString(Ty);
  Str << "\t" << ActualOpcode;
  const bool IsVInst = IsVector || IsScalarFloat;
  if (IsVInst) {
    Str << getPredicate() << WidthString;
  } else {
    Str << WidthString << getPredicate();
  }
  if (IsVector)
    Str << "." << getVecElmtBitsize(Ty);
  Str << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

template <> void InstARM32Ldr::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Variable *Dest = getDest();
  const Type DestTy = Dest->getType();
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Ldr on unknown type: " + typeStdString(DestTy));
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
  case IceType_i64:
    Asm->ldr(Dest, getSrc(0), getPredicate(), Func->getTarget());
    break;
  case IceType_f32:
    Asm->vldrs(Dest, getSrc(0), getPredicate(), Func->getTarget());
    break;
  case IceType_f64:
    Asm->vldrd(Dest, getSrc(0), getPredicate(), Func->getTarget());
    break;
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32:
  case IceType_v16i1:
  case IceType_v8i1:
  case IceType_v4i1:
    Asm->vld1qr(getVecElmtBitsize(DestTy), Dest, getSrc(0), Func->getTarget());
    break;
  }
}

template <> void InstARM32Vldr1d::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Variable *Dest = getDest();
  Asm->vld1(32, Dest, getSrc(0), Func->getTarget());
}

template <> void InstARM32Vldr1q::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Variable *Dest = getDest();
  Asm->vld1(64, Dest, getSrc(0), Func->getTarget());
}

template <> void InstARM32Ldrex::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Variable *Dest = getDest();
  Type DestTy = Dest->getType();
  assert(isScalarIntegerType(DestTy));
  const char *WidthString = getWidthString(DestTy);
  Str << "\t" << Opcode << WidthString << getPredicate() << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

template <> void InstARM32Ldrex::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Variable *Dest = getDest();
  assert(isScalarIntegerType(Dest->getType()));
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->ldrex(Dest, getSrc(0), getPredicate(), Func->getTarget());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32TwoAddrGPR<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K, bool Nws>
void InstARM32UnaryopGPR<K, Nws>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <> void InstARM32Rbit::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->rbit(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Rev::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->rev(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Movw::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\t" << Opcode << getPredicate() << "\t";
  getDest()->emit(Func);
  Str << ", ";
  auto *Src0 = llvm::cast<Constant>(getSrc(0));
  if (auto *CR = llvm::dyn_cast<ConstantRelocatable>(Src0)) {
    Str << "#:lower16:";
    CR->emitWithoutPrefix(Func->getTarget());
  } else {
    Src0->emit(Func);
  }
}

template <> void InstARM32Movw::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->movw(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Movt::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Variable *Dest = getDest();
  auto *Src1 = llvm::cast<Constant>(getSrc(1));
  Str << "\t" << Opcode << getPredicate() << "\t";
  Dest->emit(Func);
  Str << ", ";
  if (auto *CR = llvm::dyn_cast<ConstantRelocatable>(Src1)) {
    Str << "#:upper16:";
    CR->emitWithoutPrefix(Func->getTarget());
  } else {
    Src1->emit(Func);
  }
}

template <> void InstARM32Movt::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->movt(getDest(), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Clz::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->clz(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Mvn::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->mvn(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Sxt::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->sxt(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <> void InstARM32Uxt::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->uxt(getDest(), getSrc(0), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32UnaryopFP<K>::emitIAS(const Cfg *Func) const {
  emitUsingTextFixup(Func);
}

template <InstARM32::InstKindARM32 K>
void InstARM32UnaryopSignAwareFP<K>::emitIAS(const Cfg *Func) const {
  InstARM32::emitUsingTextFixup(Func);
}

template <> void InstARM32Vsqrt::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Dest = getDest();
  switch (Dest->getType()) {
  case IceType_f32:
    Asm->vsqrts(Dest, getSrc(0), getPredicate());
    break;
  case IceType_f64:
    Asm->vsqrtd(Dest, getSrc(0), getPredicate());
    break;
  default:
    llvm::report_fatal_error("Vsqrt of non-floating type");
  }
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

const char *InstARM32Pop::getGPROpcode() const { return "pop"; }

const char *InstARM32Pop::getSRegOpcode() const { return "vpop"; }

Variable *InstARM32Pop::getStackReg(SizeT Index) const { return Dests[Index]; }

SizeT InstARM32Pop::getNumStackRegs() const { return Dests.size(); }

void InstARM32Pop::emitSingleGPR(const Cfg *Func, const EmitForm Form,
                                 const Variable *Reg) const {
  switch (Form) {
  case Emit_Text:
    emitGPRsAsText(Func);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->pop(Reg, CondARM32::AL);
    return;
  }
}

void InstARM32Pop::emitMultipleGPRs(const Cfg *Func, const EmitForm Form,
                                    IValueT Registers) const {
  switch (Form) {
  case Emit_Text:
    emitGPRsAsText(Func);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->popList(Registers,
                                                         CondARM32::AL);
    return;
  }
}

void InstARM32Pop::emitSRegs(const Cfg *Func, const EmitForm Form,
                             const Variable *BaseReg, SizeT RegCount) const {
  switch (Form) {
  case Emit_Text:
    emitSRegsAsText(Func, BaseReg, RegCount);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->vpop(BaseReg, RegCount,
                                                      CondARM32::AL);
    return;
  }
}

const char *InstARM32Push::getGPROpcode() const { return "push"; }

const char *InstARM32Push::getSRegOpcode() const { return "vpush"; }

Variable *InstARM32Push::getStackReg(SizeT Index) const {
  return llvm::cast<Variable>(getSrc(Index));
}

SizeT InstARM32Push::getNumStackRegs() const { return getSrcSize(); }

void InstARM32Push::emitSingleGPR(const Cfg *Func, const EmitForm Form,
                                  const Variable *Reg) const {
  switch (Form) {
  case Emit_Text:
    emitGPRsAsText(Func);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->push(Reg, CondARM32::AL);
    return;
  }
}

void InstARM32Push::emitMultipleGPRs(const Cfg *Func, const EmitForm Form,
                                     IValueT Registers) const {
  switch (Form) {
  case Emit_Text:
    emitGPRsAsText(Func);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->pushList(Registers,
                                                          CondARM32::AL);
    return;
  }
}

void InstARM32Push::emitSRegs(const Cfg *Func, const EmitForm Form,
                              const Variable *BaseReg, SizeT RegCount) const {
  switch (Form) {
  case Emit_Text:
    emitSRegsAsText(Func, BaseReg, RegCount);
    return;
  case Emit_Binary:
    Func->getAssembler<ARM32::AssemblerARM32>()->vpush(BaseReg, RegCount,
                                                       CondARM32::AL);
    return;
  }
}

void InstARM32Ret::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() > 0);
  auto *LR = llvm::cast<Variable>(getSrc(0));
  assert(LR->hasReg());
  assert(LR->getRegNum() == RegARM32::Reg_lr);
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t"
         "bx"
         "\t";
  LR->emit(Func);
}

void InstARM32Ret::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->bx(RegARM32::Encoded_Reg_lr);
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Ret::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Type Ty = (getSrcSize() == 1 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Ty << " ";
  dumpSources(Func);
}

void InstARM32Str::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Type Ty = getSrc(0)->getType();
  const bool IsVectorStore = isVectorType(Ty);
  const bool IsScalarFloat = isScalarFloatingType(Ty);
  const char *Opcode =
      IsVectorStore ? "vst1" : (IsScalarFloat ? "vstr" : "str");
  Str << "\t" << Opcode;
  const bool IsVInst = IsVectorStore || IsScalarFloat;
  if (IsVInst) {
    Str << getPredicate() << getWidthString(Ty);
  } else {
    Str << getWidthString(Ty) << getPredicate();
  }
  if (IsVectorStore)
    Str << "." << getVecElmtBitsize(Ty);
  Str << "\t";
  getSrc(0)->emit(Func);
  Str << ", ";
  getSrc(1)->emit(Func);
}

void InstARM32Str::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  const Operand *Src1 = getSrc(1);
  Type Ty = Src0->getType();
  switch (Ty) {
  default:
    llvm::report_fatal_error("Str on unknown type: " + typeStdString(Ty));
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
  case IceType_i64:
    Asm->str(Src0, Src1, getPredicate(), Func->getTarget());
    break;
  case IceType_f32:
    Asm->vstrs(Src0, Src1, getPredicate(), Func->getTarget());
    break;
  case IceType_f64:
    Asm->vstrd(Src0, Src1, getPredicate(), Func->getTarget());
    break;
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32:
  case IceType_v16i1:
  case IceType_v8i1:
  case IceType_v4i1:
    Asm->vst1qr(getVecElmtBitsize(Ty), Src0, Src1, Func->getTarget());
    break;
  }
}

void InstARM32Str::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Type Ty = getSrc(0)->getType();
  dumpOpcodePred(Str, "str", Ty);
  Str << " ";
  getSrc(1)->dump(Func);
  Str << ", ";
  getSrc(0)->dump(Func);
}

void InstARM32Strex::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() == 2);
  Type Ty = getSrc(0)->getType();
  assert(isScalarIntegerType(Ty));
  Variable *Dest = getDest();
  Ostream &Str = Func->getContext()->getStrEmit();
  static constexpr char Opcode[] = "strex";
  const char *WidthString = getWidthString(Ty);
  Str << "\t" << Opcode << WidthString << getPredicate() << "\t";
  Dest->emit(Func);
  Str << ", ";
  emitSources(Func);
}

void InstARM32Strex::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  const Operand *Src0 = getSrc(0);
  assert(isScalarIntegerType(Src0->getType()));
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->strex(Dest, Src0, getSrc(1), getPredicate(), Func->getTarget());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Strex::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Variable *Dest = getDest();
  Dest->dump(Func);
  Str << " = ";
  Type Ty = getSrc(0)->getType();
  dumpOpcodePred(Str, "strex", Ty);
  Str << " ";
  getSrc(1)->dump(Func);
  Str << ", ";
  getSrc(0)->dump(Func);
}

void InstARM32Vstr1::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Type Ty = getSrc(0)->getType();
  const bool IsVectorStore = isVectorType(Ty);
  const bool IsScalarFloat = isScalarFloatingType(Ty);
  const char *Opcode =
      IsVectorStore ? "vst1" : (IsScalarFloat ? "vstr" : "str");
  Str << "\t" << Opcode;
  const bool IsVInst = IsVectorStore || IsScalarFloat;
  if (IsVInst) {
    Str << getPredicate() << getWidthString(Ty);
  } else {
    Str << getWidthString(Ty) << getPredicate();
  }
  if (IsVectorStore)
    Str << "." << getVecElmtBitsize(Ty);
  Str << "\t";
  getSrc(0)->emit(Func);
  Str << ", ";
  getSrc(1)->emit(Func);
}

void InstARM32Vstr1::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Src0 = getSrc(0);
  const Operand *Src1 = getSrc(1);
  Asm->vst1(Size, Src0, Src1, Func->getTarget());
}

void InstARM32Vstr1::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Type Ty = getSrc(0)->getType();
  dumpOpcodePred(Str, "str", Ty);
  Str << " ";
  getSrc(1)->dump(Func);
  Str << ", ";
  getSrc(0)->dump(Func);
}

void InstARM32Vdup::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Type Ty = getSrc(0)->getType();
  const char *Opcode = "vdup";
  Str << "\t" << Opcode;
  Str << getPredicate() << "." << getWidthString(Ty) << getVecElmtBitsize(Ty);
  Str << "\t";
  getSrc(0)->emit(Func);
  Str << ", ";
  getSrc(1)->emit(Func);
  Str << ", " << Idx;
}

void InstARM32Vdup::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Operand *Dest = getDest();
  const Operand *Src = getSrc(0);
  Type DestTy = Dest->getType();
  Asm->vdup(typeElementType(DestTy), Dest, Src, Idx);
}

void InstARM32Vdup::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = ";
  dumpOpcodePred(Str, "vdup", getDest()->getType());
  Str << " ";
  dumpSources(Func);
  Str << ", " << Idx;
}

void InstARM32Trap::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 0);
  // There isn't a mnemonic for the special NaCl Trap encoding, so dump
  // the raw bytes.
  Str << "\t.long 0x";
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  for (uint8_t I : Asm->getNonExecBundlePadding()) {
    Str.write_hex(I);
  }
}

void InstARM32Trap::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->trap();
  assert(!Asm->needsTextFixup());
}

void InstARM32Trap::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "trap";
}

void InstARM32Umull::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  assert(getDest()->hasReg());
  Str << "\t"
         "umull"
      << getPredicate() << "\t";
  getDest()->emit(Func);
  Str << ", ";
  DestHi->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
  Str << ", ";
  getSrc(1)->emit(Func);
}

void InstARM32Umull::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->umull(getDest(), DestHi, getSrc(0), getSrc(1), getPredicate());
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Umull::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = ";
  dumpOpcodePred(Str, "umull", getDest()->getType());
  Str << " ";
  dumpSources(Func);
}

namespace {
const char *vcvtVariantSuffix(const InstARM32Vcvt::VcvtVariant Variant) {
  switch (Variant) {
  case InstARM32Vcvt::S2si:
    return ".s32.f32";
  case InstARM32Vcvt::S2ui:
    return ".u32.f32";
  case InstARM32Vcvt::Si2s:
    return ".f32.s32";
  case InstARM32Vcvt::Ui2s:
    return ".f32.u32";
  case InstARM32Vcvt::D2si:
    return ".s32.f64";
  case InstARM32Vcvt::D2ui:
    return ".u32.f64";
  case InstARM32Vcvt::Si2d:
    return ".f64.s32";
  case InstARM32Vcvt::Ui2d:
    return ".f64.u32";
  case InstARM32Vcvt::S2d:
    return ".f64.f32";
  case InstARM32Vcvt::D2s:
    return ".f32.f64";
  case InstARM32Vcvt::Vs2si:
    return ".s32.f32";
  case InstARM32Vcvt::Vs2ui:
    return ".u32.f32";
  case InstARM32Vcvt::Vsi2s:
    return ".f32.s32";
  case InstARM32Vcvt::Vui2s:
    return ".f32.u32";
  }
  llvm::report_fatal_error("Invalid VcvtVariant enum.");
}
} // end of anonymous namespace

void InstARM32Vcvt::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  assert(getDest()->hasReg());
  Str << "\t"
         "vcvt"
      << getPredicate() << vcvtVariantSuffix(Variant) << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

void InstARM32Vcvt::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  switch (Variant) {
  case S2si:
    Asm->vcvtis(getDest(), getSrc(0), getPredicate());
    break;
  case S2ui:
    Asm->vcvtus(getDest(), getSrc(0), getPredicate());
    break;
  case Si2s:
    Asm->vcvtsi(getDest(), getSrc(0), getPredicate());
    break;
  case Ui2s:
    Asm->vcvtsu(getDest(), getSrc(0), getPredicate());
    break;
  case D2si:
    Asm->vcvtid(getDest(), getSrc(0), getPredicate());
    break;
  case D2ui:
    Asm->vcvtud(getDest(), getSrc(0), getPredicate());
    break;
  case Si2d:
    Asm->vcvtdi(getDest(), getSrc(0), getPredicate());
    break;
  case Ui2d:
    Asm->vcvtdu(getDest(), getSrc(0), getPredicate());
    break;
  case S2d:
    Asm->vcvtds(getDest(), getSrc(0), getPredicate());
    break;
  case D2s:
    Asm->vcvtsd(getDest(), getSrc(0), getPredicate());
    break;
  case Vs2si:
    Asm->vcvtqsi(getDest(), getSrc(0));
    break;
  case Vs2ui:
    Asm->vcvtqsu(getDest(), getSrc(0));
    break;
  case Vsi2s:
    Asm->vcvtqis(getDest(), getSrc(0));
    break;
  case Vui2s:
    Asm->vcvtqus(getDest(), getSrc(0));
    break;
  }
  assert(!Asm->needsTextFixup());
}

void InstARM32Vcvt::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = "
      << "vcvt" << getPredicate() << vcvtVariantSuffix(Variant) << " ";
  dumpSources(Func);
}

void InstARM32Vcmp::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Str << "\t"
         "vcmp"
      << getPredicate() << getFpWidthString(getSrc(0)->getType()) << "\t";
  getSrc(0)->emit(Func);
  Str << ", ";
  getSrc(1)->emit(Func);
}

void InstARM32Vcmp::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 2);
  const Operand *Src0 = getSrc(0);
  const Type Ty = Src0->getType();
  const Operand *Src1 = getSrc(1);
  const CondARM32::Cond Cond = getPredicate();
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  if (llvm::isa<OperandARM32FlexFpZero>(Src1)) {
    switch (Ty) {
    case IceType_f32:
      Asm->vcmpsz(Src0, Cond);
      break;
    case IceType_f64:
      Asm->vcmpdz(Src0, Cond);
      break;
    default:
      llvm::report_fatal_error("Vcvt on non floating value");
    }
  } else {
    switch (Ty) {
    case IceType_f32:
      Asm->vcmps(Src0, Src1, Cond);
      break;
    case IceType_f64:
      Asm->vcmpd(Src0, Src1, Cond);
      break;
    default:
      llvm::report_fatal_error("Vcvt on non floating value");
    }
  }
  assert(!Asm->needsTextFixup());
}

void InstARM32Vcmp::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "vcmp" << getPredicate() << getFpWidthString(getSrc(0)->getType());
  dumpSources(Func);
}

void InstARM32Vmrs::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 0);
  Str << "\t"
         "vmrs"
      << getPredicate()
      << "\t"
         "APSR_nzcv"
         ", "
         "FPSCR";
}

void InstARM32Vmrs::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  Asm->vmrsAPSR_nzcv(getPredicate());
  assert(!Asm->needsTextFixup());
}

void InstARM32Vmrs::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "APSR{n,z,v,c} = vmrs" << getPredicate()
      << "\t"
         "FPSCR{n,z,c,v}";
}

void InstARM32Vabs::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\t"
         "vabs"
      << getPredicate() << getFpWidthString(getSrc(0)->getType()) << "\t";
  getDest()->emit(Func);
  Str << ", ";
  getSrc(0)->emit(Func);
}

void InstARM32Vabs::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  const Variable *Dest = getDest();
  switch (Dest->getType()) {
  default:
    llvm::report_fatal_error("fabs not defined on type " +
                             typeStdString(Dest->getType()));
  case IceType_f32:
    Asm->vabss(Dest, getSrc(0), getPredicate());
    break;
  case IceType_f64:
    Asm->vabsd(Dest, getSrc(0), getPredicate());
    break;
  case IceType_v4f32:
    assert(CondARM32::isUnconditional(getPredicate()) &&
           "fabs must be unconditional");
    Asm->vabsq(Dest, getSrc(0));
  }
  assert(!Asm->needsTextFixup());
}

void InstARM32Vabs::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = vabs" << getPredicate() << getFpWidthString(getSrc(0)->getType());
}

void InstARM32Dmb::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 0);
  Str << "\t"
         "dmb"
         "\t"
         "sy";
}

void InstARM32Dmb::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 0);
  auto *Asm = Func->getAssembler<ARM32::AssemblerARM32>();
  constexpr ARM32::IValueT SyOption = 0xF; // i.e. 1111
  Asm->dmb(SyOption);
  if (Asm->needsTextFixup())
    emitUsingTextFixup(Func);
}

void InstARM32Dmb::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Func->getContext()->getStrDump() << "dmb\t"
                                      "sy";
}

void InstARM32Nop::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() == 0);
  Func->getContext()->getStrEmit() << "\t"
                                   << "nop";
}

void InstARM32Nop::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 0);
  Func->getAssembler<ARM32::AssemblerARM32>()->nop();
}

void InstARM32Nop::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() == 0);
  Func->getContext()->getStrDump() << "nop";
}

void OperandARM32Mem::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "[";
  getBase()->emit(Func);
  switch (getAddrMode()) {
  case PostIndex:
  case NegPostIndex:
    Str << "]";
    break;
  default:
    break;
  }
  if (isRegReg()) {
    Str << ", ";
    if (isNegAddrMode()) {
      Str << "-";
    }
    getIndex()->emit(Func);
    if (getShiftOp() != kNoShift) {
      Str << ", " << InstARM32ShiftAttributes[getShiftOp()].EmitString << " #"
          << getShiftAmt();
    }
  } else {
    ConstantInteger32 *Offset = getOffset();
    if (Offset && Offset->getValue() != 0) {
      Str << ", ";
      Offset->emit(Func);
    }
  }
  switch (getAddrMode()) {
  case Offset:
  case NegOffset:
    Str << "]";
    break;
  case PreIndex:
  case NegPreIndex:
    Str << "]!";
    break;
  case PostIndex:
  case NegPostIndex:
    // Brace is already closed off.
    break;
  }
}

void OperandARM32Mem::dump(const Cfg *Func, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  Str << "[";
  if (Func)
    getBase()->dump(Func);
  else
    getBase()->dump(Str);
  Str << ", ";
  if (isRegReg()) {
    if (isNegAddrMode()) {
      Str << "-";
    }
    if (Func)
      getIndex()->dump(Func);
    else
      getIndex()->dump(Str);
    if (getShiftOp() != kNoShift) {
      Str << ", " << InstARM32ShiftAttributes[getShiftOp()].EmitString << " #"
          << getShiftAmt();
    }
  } else {
    getOffset()->dump(Func, Str);
  }
  Str << "] AddrMode==" << getAddrMode();
}

void OperandARM32ShAmtImm::emit(const Cfg *Func) const { ShAmt->emit(Func); }

void OperandARM32ShAmtImm::dump(const Cfg *, Ostream &Str) const {
  ShAmt->dump(Str);
}

OperandARM32FlexImm *OperandARM32FlexImm::create(Cfg *Func, Type Ty,
                                                 uint32_t Imm,
                                                 uint32_t RotateAmt) {
  // The assembler wants the smallest rotation. Rotate if needed. Note: Imm is
  // an 8-bit value.
  assert(Utils::IsUint(8, Imm) &&
         "Flex immediates can only be defined on 8-bit immediates");
  while ((Imm & 0x03) == 0 && RotateAmt > 0) {
    --RotateAmt;
    Imm = Imm >> 2;
  }
  return new (Func->allocate<OperandARM32FlexImm>())
      OperandARM32FlexImm(Func, Ty, Imm, RotateAmt);
}

void OperandARM32FlexImm::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  uint32_t Imm = getImm();
  uint32_t RotateAmt = getRotateAmt();
  Str << "#" << Utils::rotateRight32(Imm, 2 * RotateAmt);
}

void OperandARM32FlexImm::dump(const Cfg * /* Func */, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  uint32_t Imm = getImm();
  uint32_t RotateAmt = getRotateAmt();
  Str << "#(" << Imm << " ror 2*" << RotateAmt << ")";
}

namespace {
static constexpr uint32_t a = 0x80;
static constexpr uint32_t b = 0x40;
static constexpr uint32_t cdefgh = 0x3F;
static constexpr uint32_t AllowedBits = a | b | cdefgh;
static_assert(AllowedBits == 0xFF,
              "Invalid mask for f32/f64 constant rematerialization.");

// There's no loss in always returning the modified immediate as float.
// TODO(jpp): returning a double causes problems when outputting the constants
// for filetype=asm. Why?
float materializeFloatImmediate(uint32_t ModifiedImm) {
  const uint32_t Ret = ((ModifiedImm & a) ? 0x80000000 : 0) |
                       ((ModifiedImm & b) ? 0x3E000000 : 0x40000000) |
                       ((ModifiedImm & cdefgh) << 19);
  return Utils::bitCopy<float>(Ret);
}

} // end of anonymous namespace

void OperandARM32FlexFpImm::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  switch (Ty) {
  default:
    llvm::report_fatal_error("Invalid flex fp imm type.");
  case IceType_f64:
  case IceType_f32:
    Str << "#" << materializeFloatImmediate(ModifiedImm)
        << " @ Modified: " << ModifiedImm;
    break;
  }
}

void OperandARM32FlexFpImm::dump(const Cfg * /*Func*/, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  Str << "#" << materializeFloatImmediate(ModifiedImm) << getFpWidthString(Ty);
}

void OperandARM32FlexFpZero::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  switch (Ty) {
  default:
    llvm::report_fatal_error("Invalid flex fp imm type.");
  case IceType_f64:
  case IceType_f32:
    Str << "#0.0";
  }
}

void OperandARM32FlexFpZero::dump(const Cfg * /*Func*/, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  Str << "#0.0" << getFpWidthString(Ty);
}

void OperandARM32FlexReg::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  getReg()->emit(Func);
  if (getShiftOp() != kNoShift) {
    Str << ", " << InstARM32ShiftAttributes[getShiftOp()].EmitString << " ";
    getShiftAmt()->emit(Func);
  }
}

void OperandARM32FlexReg::dump(const Cfg *Func, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  Variable *Reg = getReg();
  if (Func)
    Reg->dump(Func);
  else
    Reg->dump(Str);
  if (getShiftOp() != kNoShift) {
    Str << ", " << InstARM32ShiftAttributes[getShiftOp()].EmitString << " ";
    if (Func)
      getShiftAmt()->dump(Func);
    else
      getShiftAmt()->dump(Str);
  }
}

// Force instantition of template classes
template class InstARM32ThreeAddrGPR<InstARM32::Adc>;
template class InstARM32ThreeAddrGPR<InstARM32::Add>;
template class InstARM32ThreeAddrGPR<InstARM32::And>;
template class InstARM32ThreeAddrGPR<InstARM32::Asr>;
template class InstARM32ThreeAddrGPR<InstARM32::Bic>;
template class InstARM32ThreeAddrGPR<InstARM32::Eor>;
template class InstARM32ThreeAddrGPR<InstARM32::Lsl>;
template class InstARM32ThreeAddrGPR<InstARM32::Lsr>;
template class InstARM32ThreeAddrGPR<InstARM32::Mul>;
template class InstARM32ThreeAddrGPR<InstARM32::Orr>;
template class InstARM32ThreeAddrGPR<InstARM32::Rsb>;
template class InstARM32ThreeAddrGPR<InstARM32::Rsc>;
template class InstARM32ThreeAddrGPR<InstARM32::Sbc>;
template class InstARM32ThreeAddrGPR<InstARM32::Sdiv>;
template class InstARM32ThreeAddrGPR<InstARM32::Sub>;
template class InstARM32ThreeAddrGPR<InstARM32::Udiv>;

template class InstARM32ThreeAddrFP<InstARM32::Vadd>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vcge>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vcgt>;
template class InstARM32ThreeAddrFP<InstARM32::Vdiv>;
template class InstARM32ThreeAddrFP<InstARM32::Veor>;
template class InstARM32FourAddrFP<InstARM32::Vmla>;
template class InstARM32FourAddrFP<InstARM32::Vmls>;
template class InstARM32ThreeAddrFP<InstARM32::Vmul>;
template class InstARM32UnaryopSignAwareFP<InstARM32::Vneg>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vshl>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vshr>;
template class InstARM32ThreeAddrFP<InstARM32::Vsub>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vqadd>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vqsub>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vqmovn2>;
template class InstARM32ThreeAddrSignAwareFP<InstARM32::Vmulh>;
template class InstARM32ThreeAddrFP<InstARM32::Vmlap>;

template class InstARM32LoadBase<InstARM32::Ldr>;
template class InstARM32LoadBase<InstARM32::Ldrex>;
template class InstARM32LoadBase<InstARM32::Vldr1d>;
template class InstARM32LoadBase<InstARM32::Vldr1q>;
template class InstARM32ThreeAddrFP<InstARM32::Vzip>;
template class InstARM32TwoAddrGPR<InstARM32::Movt>;

template class InstARM32UnaryopGPR<InstARM32::Movw, false>;
template class InstARM32UnaryopGPR<InstARM32::Clz, false>;
template class InstARM32UnaryopGPR<InstARM32::Mvn, false>;
template class InstARM32UnaryopGPR<InstARM32::Rbit, false>;
template class InstARM32UnaryopGPR<InstARM32::Rev, false>;
template class InstARM32UnaryopGPR<InstARM32::Sxt, true>;
template class InstARM32UnaryopGPR<InstARM32::Uxt, true>;
template class InstARM32UnaryopFP<InstARM32::Vsqrt>;

template class InstARM32FourAddrGPR<InstARM32::Mla>;
template class InstARM32FourAddrGPR<InstARM32::Mls>;

template class InstARM32CmpLike<InstARM32::Cmn>;
template class InstARM32CmpLike<InstARM32::Cmp>;
template class InstARM32CmpLike<InstARM32::Tst>;

} // end of namespace ARM32
} // end of namespace Ice
