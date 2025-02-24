//===- subzero/src/IceAssemblerMIPS32.cpp - MIPS32 Assembler --------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Assembler class for MIPS32.
///
//===----------------------------------------------------------------------===//

#include "IceAssemblerMIPS32.h"
#include "IceCfgNode.h"
#include "IceRegistersMIPS32.h"
#include "IceUtils.h"

namespace {

using namespace Ice;
using namespace Ice::MIPS32;

// Offset modifier to current PC for next instruction.
static constexpr IOffsetT kPCReadOffset = 4;

// Mask to pull out PC offset from branch instruction.
static constexpr int kBranchOffsetBits = 16;
static constexpr IOffsetT kBranchOffsetMask = 0x0000ffff;

} // end of anonymous namespace

namespace Ice {
namespace MIPS32 {

void AssemblerMIPS32::emitTextInst(const std::string &Text, SizeT InstSize) {
  AssemblerFixup *F = createTextFixup(Text, InstSize);
  emitFixup(F);
  for (SizeT I = 0; I < InstSize; ++I) {
    AssemblerBuffer::EnsureCapacity ensured(&Buffer);
    Buffer.emit<char>(0);
  }
}

namespace {

// TEQ $0, $0 - Trap if equal
static constexpr uint8_t TrapBytesRaw[] = {0x00, 0x00, 0x00, 0x34};

const auto TrapBytes =
    llvm::ArrayRef<uint8_t>(TrapBytesRaw, llvm::array_lengthof(TrapBytesRaw));

} // end of anonymous namespace

llvm::ArrayRef<uint8_t> AssemblerMIPS32::getNonExecBundlePadding() const {
  return TrapBytes;
}

void AssemblerMIPS32::trap() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  for (const uint8_t &Byte : reverse_range(TrapBytes))
    Buffer.emit<uint8_t>(Byte);
}

void AssemblerMIPS32::nop() { emitInst(0); }

void AssemblerMIPS32::padWithNop(intptr_t Padding) {
  constexpr intptr_t InstWidth = sizeof(IValueT);
  assert(Padding % InstWidth == 0 &&
         "Padding not multiple of instruction size");
  for (intptr_t i = 0; i < Padding; i += InstWidth)
    nop();
}

Label *AssemblerMIPS32::getOrCreateLabel(SizeT Number, LabelVector &Labels) {
  Label *L = nullptr;
  if (Number == Labels.size()) {
    L = new (this->allocate<Label>()) Label();
    Labels.push_back(L);
    return L;
  }
  if (Number > Labels.size()) {
    Labels.resize(Number + 1);
  }
  L = Labels[Number];
  if (L == nullptr) {
    L = new (this->allocate<Label>()) Label();
    Labels[Number] = L;
  }
  return L;
}

void AssemblerMIPS32::bindCfgNodeLabel(const CfgNode *Node) {
  if (BuildDefs::dump() && !getFlags().getDisableHybridAssembly()) {
    constexpr SizeT InstSize = 0;
    emitTextInst(Node->getAsmName() + ":", InstSize);
  }
  SizeT NodeNumber = Node->getIndex();
  assert(!getPreliminary());
  Label *L = getOrCreateCfgNodeLabel(NodeNumber);
  this->bind(L);
}

namespace {

// Checks that Offset can fit in imm16 constant of branch instruction.
void assertCanEncodeBranchOffset(IOffsetT Offset) {
  (void)Offset;
  (void)kBranchOffsetBits;
  assert(Utils::IsAligned(Offset, 4));
  assert(Utils::IsInt(kBranchOffsetBits, Offset >> 2));
}

IValueT encodeBranchOffset(IOffsetT Offset, IValueT Inst) {
  Offset -= kPCReadOffset;
  assertCanEncodeBranchOffset(Offset);
  Offset >>= 2;
  Offset &= kBranchOffsetMask;
  return (Inst & ~kBranchOffsetMask) | Offset;
}

enum RegSetWanted { WantGPRegs, WantFPRegs };

IValueT getEncodedGPRegNum(const Variable *Var) {
  assert(Var->hasReg() && isScalarIntegerType(Var->getType()));
  const auto Reg = Var->getRegNum();
  return RegMIPS32::getEncodedGPR(Reg);
}

IValueT getEncodedFPRegNum(const Variable *Var) {
  assert(Var->hasReg() && isScalarFloatingType(Var->getType()));
  const auto Reg = Var->getRegNum();
  IValueT RegEncoding;
  if (RegMIPS32::isFPRReg(Reg)) {
    RegEncoding = RegMIPS32::getEncodedFPR(Reg);
  } else {
    RegEncoding = RegMIPS32::getEncodedFPR64(Reg);
  }
  return RegEncoding;
}

bool encodeOperand(const Operand *Opnd, IValueT &Value,
                   RegSetWanted WantedRegSet) {
  Value = 0;
  if (const auto *Var = llvm::dyn_cast<Variable>(Opnd)) {
    if (Var->hasReg()) {
      switch (WantedRegSet) {
      case WantGPRegs:
        Value = getEncodedGPRegNum(Var);
        break;
      case WantFPRegs:
        Value = getEncodedFPRegNum(Var);
        break;
      }
      return true;
    }
    return false;
  }
  return false;
}

IValueT encodeRegister(const Operand *OpReg, RegSetWanted WantedRegSet,
                       const char *RegName, const char *InstName) {
  IValueT Reg = 0;
  if (encodeOperand(OpReg, Reg, WantedRegSet) != true)
    llvm::report_fatal_error(std::string(InstName) + ": Can't find register " +
                             RegName);
  return Reg;
}

IValueT encodeGPRegister(const Operand *OpReg, const char *RegName,
                         const char *InstName) {
  return encodeRegister(OpReg, WantGPRegs, RegName, InstName);
}

IValueT encodeFPRegister(const Operand *OpReg, const char *RegName,
                         const char *InstName) {
  return encodeRegister(OpReg, WantFPRegs, RegName, InstName);
}

} // end of anonymous namespace

IOffsetT AssemblerMIPS32::decodeBranchOffset(IValueT Inst) {
  int16_t imm = (Inst & kBranchOffsetMask);
  IOffsetT Offset = imm;
  Offset = Offset << 2;
  return (Offset + kPCReadOffset);
}

void AssemblerMIPS32::bind(Label *L) {
  IOffsetT BoundPc = Buffer.size();
  assert(!L->isBound()); // Labels can only be bound once.
  while (L->isLinked()) {
    IOffsetT Position = L->getLinkPosition();
    IOffsetT Dest = BoundPc - Position;
    IValueT Inst = Buffer.load<IValueT>(Position);
    Buffer.store<IValueT>(Position, encodeBranchOffset(Dest, Inst));
    IOffsetT NextBrPc = decodeBranchOffset(Inst);
    if (NextBrPc != 0)
      NextBrPc = Position - NextBrPc;
    L->setPosition(NextBrPc);
  }
  L->bindTo(BoundPc);
}

void AssemblerMIPS32::emitRsRt(IValueT Opcode, const Operand *OpRs,
                               const Operand *OpRt, const char *InsnName) {
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", InsnName);
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);

  Opcode |= Rs << 21;
  Opcode |= Rt << 16;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitRtRsImm16(IValueT Opcode, const Operand *OpRt,
                                    const Operand *OpRs, const uint32_t Imm,
                                    const char *InsnName) {
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", InsnName);

  Opcode |= Rs << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm & 0xffff;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitRtRsImm16Rel(IValueT Opcode, const Operand *OpRt,
                                       const Operand *OpRs,
                                       const Operand *OpImm,
                                       const RelocOp Reloc,
                                       const char *InsnName) {
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", InsnName);
  uint32_t Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpImm)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpImm)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error(std::string(InsnName) + ": Invalid 3rd operand");
  }

  Opcode |= Rs << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm16 & 0xffff;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitFtRsImm16(IValueT Opcode, const Operand *OpFt,
                                    const Operand *OpRs, const uint32_t Imm,
                                    const char *InsnName) {
  const IValueT Ft = encodeFPRegister(OpFt, "Ft", InsnName);
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", InsnName);

  Opcode |= Rs << 21;
  Opcode |= Ft << 16;
  Opcode |= Imm & 0xffff;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitRdRtSa(IValueT Opcode, const Operand *OpRd,
                                 const Operand *OpRt, const uint32_t Sa,
                                 const char *InsnName) {
  const IValueT Rd = encodeGPRegister(OpRd, "Rd", InsnName);
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);

  Opcode |= Rt << 16;
  Opcode |= Rd << 11;
  Opcode |= (Sa & 0x1f) << 6;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitRdRsRt(IValueT Opcode, const Operand *OpRd,
                                 const Operand *OpRs, const Operand *OpRt,
                                 const char *InsnName) {
  const IValueT Rd = encodeGPRegister(OpRd, "Rd", InsnName);
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", InsnName);
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);

  Opcode |= Rs << 21;
  Opcode |= Rt << 16;
  Opcode |= Rd << 11;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitCOP1Fcmp(IValueT Opcode, FPInstDataFormat Format,
                                   const Operand *OpFs, const Operand *OpFt,
                                   IValueT CC, const char *InsnName) {
  const IValueT Fs = encodeFPRegister(OpFs, "Fs", InsnName);
  const IValueT Ft = encodeFPRegister(OpFt, "Ft", InsnName);

  Opcode |= CC << 8;
  Opcode |= Fs << 11;
  Opcode |= Ft << 16;
  Opcode |= Format << 21;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitCOP1FmtFsFd(IValueT Opcode, FPInstDataFormat Format,
                                      const Operand *OpFd, const Operand *OpFs,
                                      const char *InsnName) {
  const IValueT Fd = encodeFPRegister(OpFd, "Fd", InsnName);
  const IValueT Fs = encodeFPRegister(OpFs, "Fs", InsnName);

  Opcode |= Fd << 6;
  Opcode |= Fs << 11;
  Opcode |= Format << 21;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitCOP1FmtFtFsFd(IValueT Opcode, FPInstDataFormat Format,
                                        const Operand *OpFd,
                                        const Operand *OpFs,
                                        const Operand *OpFt,
                                        const char *InsnName) {
  const IValueT Fd = encodeFPRegister(OpFd, "Fd", InsnName);
  const IValueT Fs = encodeFPRegister(OpFs, "Fs", InsnName);
  const IValueT Ft = encodeFPRegister(OpFt, "Ft", InsnName);

  Opcode |= Fd << 6;
  Opcode |= Fs << 11;
  Opcode |= Ft << 16;
  Opcode |= Format << 21;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitCOP1FmtRtFsFd(IValueT Opcode, FPInstDataFormat Format,
                                        const Operand *OpFd,
                                        const Operand *OpFs,
                                        const Operand *OpRt,
                                        const char *InsnName) {
  const IValueT Fd = encodeFPRegister(OpFd, "Fd", InsnName);
  const IValueT Fs = encodeFPRegister(OpFs, "Fs", InsnName);
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);

  Opcode |= Fd << 6;
  Opcode |= Fs << 11;
  Opcode |= Rt << 16;
  Opcode |= Format << 21;

  emitInst(Opcode);
}

void AssemblerMIPS32::emitCOP1MovRtFs(IValueT Opcode, const Operand *OpRt,
                                      const Operand *OpFs,
                                      const char *InsnName) {
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InsnName);
  const IValueT Fs = encodeFPRegister(OpFs, "Fs", InsnName);
  Opcode |= Fs << 11;
  Opcode |= Rt << 16;

  emitInst(Opcode);
}

void AssemblerMIPS32::abs_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000005;
  emitCOP1FmtFsFd(Opcode, DoublePrecision, OpFd, OpFs, "abs.d");
}

void AssemblerMIPS32::abs_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000005;
  emitCOP1FmtFsFd(Opcode, SinglePrecision, OpFd, OpFs, "abs.s");
}

void AssemblerMIPS32::addi(const Operand *OpRt, const Operand *OpRs,
                           const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x20000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "addi");
}

void AssemblerMIPS32::add_d(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000000;
  emitCOP1FmtFtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "add.d");
}

void AssemblerMIPS32::add_s(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000000;
  emitCOP1FmtFtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "add.s");
}

void AssemblerMIPS32::addiu(const Operand *OpRt, const Operand *OpRs,
                            const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x24000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "addiu");
}

void AssemblerMIPS32::addiu(const Operand *OpRt, const Operand *OpRs,
                            const Operand *OpImm, const RelocOp Reloc) {
  static constexpr IValueT Opcode = 0x24000000;
  emitRtRsImm16Rel(Opcode, OpRt, OpRs, OpImm, Reloc, "addiu");
}

void AssemblerMIPS32::addu(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000021;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "addu");
}

void AssemblerMIPS32::and_(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000024;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "and");
}

void AssemblerMIPS32::andi(const Operand *OpRt, const Operand *OpRs,
                           const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x30000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "andi");
}

void AssemblerMIPS32::b(Label *TargetLabel) {
  static constexpr Operand *OpRsNone = nullptr;
  static constexpr Operand *OpRtNone = nullptr;
  if (TargetLabel->isBound()) {
    const int32_t Dest = TargetLabel->getPosition() - Buffer.size();
    emitBr(CondMIPS32::AL, OpRsNone, OpRtNone, Dest);
    return;
  }
  const IOffsetT Position = Buffer.size();
  IOffsetT PrevPosition = TargetLabel->getEncodedPosition();
  if (PrevPosition != 0)
    PrevPosition = Position - PrevPosition;
  emitBr(CondMIPS32::AL, OpRsNone, OpRtNone, PrevPosition);
  TargetLabel->linkTo(*this, Position);
}

void AssemblerMIPS32::c_eq_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000032;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.eq.d");
}

void AssemblerMIPS32::c_eq_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000032;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.eq.s");
}

void AssemblerMIPS32::c_ole_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000036;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ole.d");
}

void AssemblerMIPS32::c_ole_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000036;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ole.s");
}

void AssemblerMIPS32::c_olt_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000034;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.olt.d");
}

void AssemblerMIPS32::c_olt_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000034;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.olt.s");
}

void AssemblerMIPS32::c_ueq_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000033;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ueq.d");
}

void AssemblerMIPS32::c_ueq_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000033;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ueq.s");
}

void AssemblerMIPS32::c_ule_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000037;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ule.d");
}

void AssemblerMIPS32::c_ule_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000037;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ule.s");
}

void AssemblerMIPS32::c_ult_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000035;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ult.d");
}

void AssemblerMIPS32::c_ult_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000035;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.ult.s");
}

void AssemblerMIPS32::c_un_d(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000031;
  emitCOP1Fcmp(Opcode, DoublePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.un.d");
}

void AssemblerMIPS32::c_un_s(const Operand *OpFs, const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000031;
  emitCOP1Fcmp(Opcode, SinglePrecision, OpFs, OpFt, OperandMIPS32FCC::FCC0,
               "c.un.s");
}

void AssemblerMIPS32::clz(const Operand *OpRd, const Operand *OpRs) {
  IValueT Opcode = 0x70000020;
  const IValueT Rd = encodeGPRegister(OpRd, "Rd", "clz");
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", "clz");
  Opcode |= Rd << 11;
  Opcode |= Rd << 16;
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::cvt_d_l(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000021;
  emitCOP1FmtFsFd(Opcode, Long, OpFd, OpFs, "cvt.d.l");
}

void AssemblerMIPS32::cvt_d_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000021;
  emitCOP1FmtFsFd(Opcode, SinglePrecision, OpFd, OpFs, "cvt.d.s");
}

void AssemblerMIPS32::cvt_d_w(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000021;
  emitCOP1FmtFsFd(Opcode, Word, OpFd, OpFs, "cvt.d.w");
}

void AssemblerMIPS32::cvt_s_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000020;
  emitCOP1FmtFsFd(Opcode, DoublePrecision, OpFd, OpFs, "cvt.s.d");
}

void AssemblerMIPS32::cvt_s_l(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000020;
  emitCOP1FmtFsFd(Opcode, Long, OpFd, OpFs, "cvt.s.l");
}

void AssemblerMIPS32::cvt_s_w(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000020;
  emitCOP1FmtFsFd(Opcode, Word, OpFd, OpFs, "cvt.s.w");
}

void AssemblerMIPS32::div(const Operand *OpRs, const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000001A;
  emitRsRt(Opcode, OpRs, OpRt, "div");
}

void AssemblerMIPS32::div_d(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000003;
  emitCOP1FmtFtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "div.d");
}

void AssemblerMIPS32::div_s(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000003;
  emitCOP1FmtFtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "div.s");
}

void AssemblerMIPS32::divu(const Operand *OpRs, const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000001B;
  emitRsRt(Opcode, OpRs, OpRt, "divu");
}

MIPS32Fixup *AssemblerMIPS32::createMIPS32Fixup(const RelocOp Reloc,
                                                const Constant *RelOp) {
  MIPS32Fixup *Fixup = new (allocate<MIPS32Fixup>()) MIPS32Fixup();
  switch (Reloc) {
  case RelocOp::RO_Hi:
    Fixup->set_kind(llvm::ELF::R_MIPS_HI16);
    break;
  case RelocOp::RO_Lo:
    Fixup->set_kind(llvm::ELF::R_MIPS_LO16);
    break;
  case RelocOp::RO_Jal:
    Fixup->set_kind(llvm::ELF::R_MIPS_26);
    break;
  default:
    llvm::report_fatal_error("Fixup: Invalid Reloc type");
    break;
  }
  Fixup->set_value(RelOp);
  Buffer.installFixup(Fixup);
  return Fixup;
}

size_t MIPS32Fixup::emit(GlobalContext *Ctx, const Assembler &Asm) const {
  if (!BuildDefs::dump())
    return InstMIPS32::InstSize;
  Ostream &Str = Ctx->getStrEmit();
  IValueT Inst = Asm.load<IValueT>(position());
  const auto Symbol = symbol().toString();
  Str << "\t"
      << ".word " << llvm::format_hex(Inst, 8) << " # ";
  switch (kind()) {
  case llvm::ELF::R_MIPS_HI16:
    Str << "R_MIPS_HI16 ";
    break;
  case llvm::ELF::R_MIPS_LO16:
    Str << "R_MIPS_LO16 ";
    break;
  case llvm::ELF::R_MIPS_26:
    Str << "R_MIPS_26 ";
    break;
  default:
    Str << "Unknown ";
    break;
  }
  Str << Symbol << "\n";
  return InstMIPS32::InstSize;
}

void MIPS32Fixup::emitOffset(Assembler *Asm) const {
  const IValueT Inst = Asm->load<IValueT>(position());
  IValueT ImmMask = 0;
  const IValueT Imm = offset();
  if (kind() == llvm::ELF::R_MIPS_26) {
    ImmMask = 0x03FFFFFF;
  } else {
    ImmMask = 0x0000FFFF;
  }
  Asm->store(position(), (Inst & ~ImmMask) | (Imm & ImmMask));
}

void AssemblerMIPS32::jal(const ConstantRelocatable *Target) {
  IValueT Opcode = 0x0C000000;
  emitFixup(createMIPS32Fixup(RelocOp::RO_Jal, Target));
  emitInst(Opcode);
  nop();
}

void AssemblerMIPS32::jalr(const Operand *OpRs, const Operand *OpRd) {
  IValueT Opcode = 0x00000009;
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", "jalr");
  const IValueT Rd =
      (OpRd == nullptr) ? 31 : encodeGPRegister(OpRd, "Rd", "jalr");
  Opcode |= Rd << 11;
  Opcode |= Rs << 21;
  emitInst(Opcode);
  nop();
}

void AssemblerMIPS32::lui(const Operand *OpRt, const Operand *OpImm,
                          const RelocOp Reloc) {
  IValueT Opcode = 0x3C000000;
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", "lui");
  IValueT Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpImm)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpImm)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error("lui: Invalid 2nd operand");
  }

  Opcode |= Rt << 16;
  Opcode |= Imm16;
  emitInst(Opcode);
}

void AssemblerMIPS32::ldc1(const Operand *OpRt, const Operand *OpBase,
                           const Operand *OpOff, const RelocOp Reloc) {
  IValueT Opcode = 0xD4000000;
  const IValueT Rt = encodeFPRegister(OpRt, "Ft", "ldc1");
  const IValueT Base = encodeGPRegister(OpBase, "Base", "ldc1");
  IValueT Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpOff)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpOff)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error("ldc1: Invalid 2nd operand");
  }

  Opcode |= Base << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm16;
  emitInst(Opcode);
}

void AssemblerMIPS32::ll(const Operand *OpRt, const Operand *OpBase,
                         const uint32_t Offset) {
  static constexpr IValueT Opcode = 0xC0000000;
  emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "ll");
}

void AssemblerMIPS32::lw(const Operand *OpRt, const Operand *OpBase,
                         const uint32_t Offset) {
  switch (OpRt->getType()) {
  case IceType_i1:
  case IceType_i8: {
    static constexpr IValueT Opcode = 0x80000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "lb");
    break;
  }
  case IceType_i16: {
    static constexpr IValueT Opcode = 0x84000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "lh");
    break;
  }
  case IceType_i32: {
    static constexpr IValueT Opcode = 0x8C000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "lw");
    break;
  }
  case IceType_f32: {
    static constexpr IValueT Opcode = 0xC4000000;
    emitFtRsImm16(Opcode, OpRt, OpBase, Offset, "lwc1");
    break;
  }
  case IceType_f64: {
    static constexpr IValueT Opcode = 0xD4000000;
    emitFtRsImm16(Opcode, OpRt, OpBase, Offset, "ldc1");
    break;
  }
  default: {
    UnimplementedError(getFlags());
  }
  }
}

void AssemblerMIPS32::lwc1(const Operand *OpRt, const Operand *OpBase,
                           const Operand *OpOff, const RelocOp Reloc) {
  IValueT Opcode = 0xC4000000;
  const IValueT Rt = encodeFPRegister(OpRt, "Ft", "lwc1");
  const IValueT Base = encodeGPRegister(OpBase, "Base", "lwc1");
  IValueT Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpOff)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpOff)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error("lwc1: Invalid 2nd operand");
  }

  Opcode |= Base << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm16;
  emitInst(Opcode);
}

void AssemblerMIPS32::mfc1(const Operand *OpRt, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000000;
  emitCOP1MovRtFs(Opcode, OpRt, OpFs, "mfc1");
}

void AssemblerMIPS32::mfhi(const Operand *OpRd) {
  IValueT Opcode = 0x000000010;
  IValueT Rd = encodeGPRegister(OpRd, "Rd", "mfhi");
  Opcode |= Rd << 11;
  emitInst(Opcode);
}

void AssemblerMIPS32::mflo(const Operand *OpRd) {
  IValueT Opcode = 0x000000012;
  IValueT Rd = encodeGPRegister(OpRd, "Rd", "mflo");
  Opcode |= Rd << 11;
  emitInst(Opcode);
}

void AssemblerMIPS32::mov_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000006;
  emitCOP1FmtFsFd(Opcode, DoublePrecision, OpFd, OpFs, "mov.d");
}

void AssemblerMIPS32::mov_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000006;
  emitCOP1FmtFsFd(Opcode, SinglePrecision, OpFd, OpFs, "mov.s");
}

void AssemblerMIPS32::move(const Operand *OpRd, const Operand *OpRs) {

  const Type DstType = OpRd->getType();
  const Type SrcType = OpRs->getType();

  if ((isScalarIntegerType(DstType) && isScalarFloatingType(SrcType)) ||
      (isScalarFloatingType(DstType) && isScalarIntegerType(SrcType))) {
    if (isScalarFloatingType(DstType)) {
      mtc1(OpRs, OpRd);
    } else {
      mfc1(OpRd, OpRs);
    }
  } else {
    switch (DstType) {
    case IceType_f32:
      mov_s(OpRd, OpRs);
      break;
    case IceType_f64:
      mov_d(OpRd, OpRs);
      break;
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32: {
      IValueT Opcode = 0x00000021;
      const IValueT Rd = encodeGPRegister(OpRd, "Rd", "pseudo-move");
      const IValueT Rs = encodeGPRegister(OpRs, "Rs", "pseudo-move");
      const IValueT Rt = 0; // $0
      Opcode |= Rs << 21;
      Opcode |= Rt << 16;
      Opcode |= Rd << 11;
      emitInst(Opcode);
      break;
    }
    default: {
      UnimplementedError(getFlags());
    }
    }
  }
}

void AssemblerMIPS32::movf(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpCc) {
  IValueT Opcode = 0x00000001;
  const IValueT Rd = encodeGPRegister(OpRd, "Rd", "movf");
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", "movf");
  OperandMIPS32FCC::FCC Cc = OperandMIPS32FCC::FCC0;
  if (const auto *OpFCC = llvm::dyn_cast<OperandMIPS32FCC>(OpCc)) {
    Cc = OpFCC->getFCC();
  }
  const IValueT InstEncodingFalse = 0;
  Opcode |= Rd << 11;
  Opcode |= InstEncodingFalse << 16;
  Opcode |= Cc << 18;
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::movn(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000000B;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "movn");
}

void AssemblerMIPS32::movn_d(const Operand *OpFd, const Operand *OpFs,
                             const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000013;
  emitCOP1FmtRtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "movn.d");
}

void AssemblerMIPS32::movn_s(const Operand *OpFd, const Operand *OpFs,
                             const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000013;
  emitCOP1FmtRtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "movn.s");
}

void AssemblerMIPS32::movt(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpCc) {
  IValueT Opcode = 0x00000001;
  const IValueT Rd = encodeGPRegister(OpRd, "Rd", "movt");
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", "movt");
  OperandMIPS32FCC::FCC Cc = OperandMIPS32FCC::FCC0;
  if (const auto *OpFCC = llvm::dyn_cast<OperandMIPS32FCC>(OpCc)) {
    Cc = OpFCC->getFCC();
  }
  const IValueT InstEncodingTrue = 1;
  Opcode |= Rd << 11;
  Opcode |= InstEncodingTrue << 16;
  Opcode |= Cc << 18;
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::movz_d(const Operand *OpFd, const Operand *OpFs,
                             const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000012;
  emitCOP1FmtFtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "movz.d");
}

void AssemblerMIPS32::movz(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000000A;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "movz");
}

void AssemblerMIPS32::movz_s(const Operand *OpFd, const Operand *OpFs,
                             const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000012;
  emitCOP1FmtFtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "movz.s");
}

void AssemblerMIPS32::mtc1(const Operand *OpRt, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44800000;
  emitCOP1MovRtFs(Opcode, OpRt, OpFs, "mtc1");
}

void AssemblerMIPS32::mthi(const Operand *OpRs) {
  IValueT Opcode = 0x000000011;
  IValueT Rs = encodeGPRegister(OpRs, "Rs", "mthi");
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::mtlo(const Operand *OpRs) {
  IValueT Opcode = 0x000000013;
  IValueT Rs = encodeGPRegister(OpRs, "Rs", "mtlo");
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::mul(const Operand *OpRd, const Operand *OpRs,
                          const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x70000002;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "mul");
}

void AssemblerMIPS32::mul_d(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000002;
  emitCOP1FmtFtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "mul.d");
}

void AssemblerMIPS32::mul_s(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000002;
  emitCOP1FmtFtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "mul.s");
}

void AssemblerMIPS32::mult(const Operand *OpRs, const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000018;
  emitRsRt(Opcode, OpRs, OpRt, "mult");
}

void AssemblerMIPS32::multu(const Operand *OpRs, const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000019;
  emitRsRt(Opcode, OpRs, OpRt, "multu");
}

void AssemblerMIPS32::nor(const Operand *OpRd, const Operand *OpRs,
                          const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000027;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "nor");
}

void AssemblerMIPS32::or_(const Operand *OpRd, const Operand *OpRs,
                          const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000025;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "or");
}

void AssemblerMIPS32::ori(const Operand *OpRt, const Operand *OpRs,
                          const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x34000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "ori");
}

void AssemblerMIPS32::ret(void) {
  static constexpr IValueT Opcode = 0x03E00008; // JR $31
  emitInst(Opcode);
  nop(); // delay slot
}

void AssemblerMIPS32::sc(const Operand *OpRt, const Operand *OpBase,
                         const uint32_t Offset) {
  static constexpr IValueT Opcode = 0xE0000000;
  emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "sc");
}

void AssemblerMIPS32::sll(const Operand *OpRd, const Operand *OpRt,
                          const uint32_t Sa) {
  static constexpr IValueT Opcode = 0x00000000;
  emitRdRtSa(Opcode, OpRd, OpRt, Sa, "sll");
}

void AssemblerMIPS32::sllv(const Operand *OpRd, const Operand *OpRt,
                           const Operand *OpRs) {
  static constexpr IValueT Opcode = 0x00000004;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "sllv");
}

void AssemblerMIPS32::slt(const Operand *OpRd, const Operand *OpRs,
                          const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000002A;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "slt");
}

void AssemblerMIPS32::slti(const Operand *OpRt, const Operand *OpRs,
                           const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x28000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "slti");
}

void AssemblerMIPS32::sltu(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x0000002B;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "sltu");
}

void AssemblerMIPS32::sltiu(const Operand *OpRt, const Operand *OpRs,
                            const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x2c000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "sltiu");
}

void AssemblerMIPS32::sqrt_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000004;
  emitCOP1FmtFsFd(Opcode, DoublePrecision, OpFd, OpFs, "sqrt.d");
}

void AssemblerMIPS32::sqrt_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x44000004;
  emitCOP1FmtFsFd(Opcode, SinglePrecision, OpFd, OpFs, "sqrt.s");
}

void AssemblerMIPS32::sra(const Operand *OpRd, const Operand *OpRt,
                          const uint32_t Sa) {
  static constexpr IValueT Opcode = 0x00000003;
  emitRdRtSa(Opcode, OpRd, OpRt, Sa, "sra");
}

void AssemblerMIPS32::srl(const Operand *OpRd, const Operand *OpRt,
                          const uint32_t Sa) {
  static constexpr IValueT Opcode = 0x00000002;
  emitRdRtSa(Opcode, OpRd, OpRt, Sa, "srl");
}

void AssemblerMIPS32::srav(const Operand *OpRd, const Operand *OpRt,
                           const Operand *OpRs) {
  static constexpr IValueT Opcode = 0x00000007;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "srav");
}

void AssemblerMIPS32::srlv(const Operand *OpRd, const Operand *OpRt,
                           const Operand *OpRs) {
  static constexpr IValueT Opcode = 0x00000006;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "srlv");
}

void AssemblerMIPS32::sub_d(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000001;
  emitCOP1FmtFtFsFd(Opcode, DoublePrecision, OpFd, OpFs, OpFt, "sub.d");
}

void AssemblerMIPS32::sub_s(const Operand *OpFd, const Operand *OpFs,
                            const Operand *OpFt) {
  static constexpr IValueT Opcode = 0x44000001;
  emitCOP1FmtFtFsFd(Opcode, SinglePrecision, OpFd, OpFs, OpFt, "sub.s");
}

void AssemblerMIPS32::subu(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000023;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "subu");
}

void AssemblerMIPS32::sdc1(const Operand *OpRt, const Operand *OpBase,
                           const Operand *OpOff, const RelocOp Reloc) {
  IValueT Opcode = 0xF4000000;
  const IValueT Rt = encodeFPRegister(OpRt, "Ft", "sdc1");
  const IValueT Base = encodeGPRegister(OpBase, "Base", "sdc1");
  IValueT Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpOff)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpOff)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error("sdc1: Invalid 2nd operand");
  }

  Opcode |= Base << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm16;
  emitInst(Opcode);
}

void AssemblerMIPS32::sw(const Operand *OpRt, const Operand *OpBase,
                         const uint32_t Offset) {
  switch (OpRt->getType()) {
  case IceType_i1:
  case IceType_i8: {
    static constexpr IValueT Opcode = 0xA0000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "sb");
    break;
  }
  case IceType_i16: {
    static constexpr IValueT Opcode = 0xA4000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "sh");
    break;
  }
  case IceType_i32: {
    static constexpr IValueT Opcode = 0xAC000000;
    emitRtRsImm16(Opcode, OpRt, OpBase, Offset, "sw");
    break;
  }
  case IceType_f32: {
    static constexpr IValueT Opcode = 0xE4000000;
    emitFtRsImm16(Opcode, OpRt, OpBase, Offset, "swc1");
    break;
  }
  case IceType_f64: {
    static constexpr IValueT Opcode = 0xF4000000;
    emitFtRsImm16(Opcode, OpRt, OpBase, Offset, "sdc1");
    break;
  }
  default: {
    UnimplementedError(getFlags());
  }
  }
}

void AssemblerMIPS32::swc1(const Operand *OpRt, const Operand *OpBase,
                           const Operand *OpOff, const RelocOp Reloc) {
  IValueT Opcode = 0xE4000000;
  const IValueT Rt = encodeFPRegister(OpRt, "Ft", "swc1");
  const IValueT Base = encodeGPRegister(OpBase, "Base", "swc1");
  IValueT Imm16 = 0;

  if (const auto *OpRel = llvm::dyn_cast<ConstantRelocatable>(OpOff)) {
    emitFixup(createMIPS32Fixup(Reloc, OpRel));
  } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(OpOff)) {
    Imm16 = C32->getValue();
  } else {
    llvm::report_fatal_error("swc1: Invalid 2nd operand");
  }

  Opcode |= Base << 21;
  Opcode |= Rt << 16;
  Opcode |= Imm16;
  emitInst(Opcode);
}

void AssemblerMIPS32::sync() {
  static constexpr IValueT Opcode = 0x0000000f;
  emitInst(Opcode);
}

void AssemblerMIPS32::teq(const Operand *OpRs, const Operand *OpRt,
                          const uint32_t TrapCode) {
  IValueT Opcode = 0x00000034;
  const IValueT Rs = encodeGPRegister(OpRs, "Rs", "teq");
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", "teq");
  Opcode |= (TrapCode & 0xFFFFF) << 6;
  Opcode |= Rt << 16;
  Opcode |= Rs << 21;
  emitInst(Opcode);
}

void AssemblerMIPS32::trunc_l_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x4400000D;
  emitCOP1FmtFsFd(Opcode, Long, OpFd, OpFs, "trunc.l.d");
}

void AssemblerMIPS32::trunc_l_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x4400000D;
  emitCOP1FmtFsFd(Opcode, Long, OpFd, OpFs, "trunc.l.s");
}

void AssemblerMIPS32::trunc_w_d(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x4400000D;
  emitCOP1FmtFsFd(Opcode, DoublePrecision, OpFd, OpFs, "trunc.w.d");
}

void AssemblerMIPS32::trunc_w_s(const Operand *OpFd, const Operand *OpFs) {
  static constexpr IValueT Opcode = 0x4400000D;
  emitCOP1FmtFsFd(Opcode, SinglePrecision, OpFd, OpFs, "trunc.w.s");
}

void AssemblerMIPS32::xor_(const Operand *OpRd, const Operand *OpRs,
                           const Operand *OpRt) {
  static constexpr IValueT Opcode = 0x00000026;
  emitRdRsRt(Opcode, OpRd, OpRs, OpRt, "xor");
}

void AssemblerMIPS32::xori(const Operand *OpRt, const Operand *OpRs,
                           const uint32_t Imm) {
  static constexpr IValueT Opcode = 0x38000000;
  emitRtRsImm16(Opcode, OpRt, OpRs, Imm, "xori");
}

void AssemblerMIPS32::emitBr(const CondMIPS32::Cond Cond, const Operand *OpRs,
                             const Operand *OpRt, IOffsetT Offset) {
  IValueT Opcode = 0;

  switch (Cond) {
  default:
    break;
  case CondMIPS32::AL:
  case CondMIPS32::EQ:
  case CondMIPS32::EQZ:
    Opcode = 0x10000000;
    break;
  case CondMIPS32::NE:
  case CondMIPS32::NEZ:
    Opcode = 0x14000000;
    break;
  case CondMIPS32::LEZ:
    Opcode = 0x18000000;
    break;
  case CondMIPS32::LTZ:
    Opcode = 0x04000000;
    break;
  case CondMIPS32::GEZ:
    Opcode = 0x04010000;
    break;
  case CondMIPS32::GTZ:
    Opcode = 0x1C000000;
    break;
  }

  if (Opcode == 0) {
    llvm::report_fatal_error("Branch: Invalid condition");
  }

  if (OpRs != nullptr) {
    IValueT Rs = encodeGPRegister(OpRs, "Rs", "branch");
    Opcode |= Rs << 21;
  }

  if (OpRt != nullptr) {
    IValueT Rt = encodeGPRegister(OpRt, "Rt", "branch");
    Opcode |= Rt << 16;
  }

  Opcode = encodeBranchOffset(Offset, Opcode);
  emitInst(Opcode);
  nop(); // delay slot
}

void AssemblerMIPS32::bcc(const CondMIPS32::Cond Cond, const Operand *OpRs,
                          const Operand *OpRt, Label *TargetLabel) {
  if (TargetLabel->isBound()) {
    const int32_t Dest = TargetLabel->getPosition() - Buffer.size();
    emitBr(Cond, OpRs, OpRt, Dest);
    return;
  }
  const IOffsetT Position = Buffer.size();
  IOffsetT PrevPosition = TargetLabel->getEncodedPosition();
  if (PrevPosition != 0)
    PrevPosition = Position - PrevPosition;
  emitBr(Cond, OpRs, OpRt, PrevPosition);
  TargetLabel->linkTo(*this, Position);
}

void AssemblerMIPS32::bzc(const CondMIPS32::Cond Cond, const Operand *OpRs,
                          Label *TargetLabel) {
  static constexpr Operand *OpRtNone = nullptr;
  if (TargetLabel->isBound()) {
    const int32_t Dest = TargetLabel->getPosition() - Buffer.size();
    emitBr(Cond, OpRs, OpRtNone, Dest);
    return;
  }
  const IOffsetT Position = Buffer.size();
  IOffsetT PrevPosition = TargetLabel->getEncodedPosition();
  if (PrevPosition)
    PrevPosition = Position - PrevPosition;
  emitBr(Cond, OpRs, OpRtNone, PrevPosition);
  TargetLabel->linkTo(*this, Position);
}

} // end of namespace MIPS32
} // end of namespace Ice
