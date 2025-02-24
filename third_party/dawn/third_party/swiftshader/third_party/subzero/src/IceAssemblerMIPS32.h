//===- subzero/src/IceAssemblerMIPS32.h - Assembler for MIPS ----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Assembler class for MIPS32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEASSEMBLERMIPS32_H
#define SUBZERO_SRC_ICEASSEMBLERMIPS32_H

#include "IceAssembler.h"
#include "IceDefs.h"
#include "IceFixups.h"
#include "IceInstMIPS32.h"
#include "IceTargetLowering.h"

namespace Ice {
namespace MIPS32 {

using IValueT = uint32_t;
using IOffsetT = int32_t;

enum FPInstDataFormat {
  SinglePrecision = 16,
  DoublePrecision = 17,
  Word = 20,
  Long = 21
};

class MIPS32Fixup final : public AssemblerFixup {
  MIPS32Fixup &operator=(const MIPS32Fixup &) = delete;
  MIPS32Fixup(const MIPS32Fixup &) = default;

public:
  MIPS32Fixup() = default;
  size_t emit(GlobalContext *Ctx, const Assembler &Asm) const final;
  void emitOffset(Assembler *Asm) const;
};

class AssemblerMIPS32 : public Assembler {
  AssemblerMIPS32(const AssemblerMIPS32 &) = delete;
  AssemblerMIPS32 &operator=(const AssemblerMIPS32 &) = delete;

public:
  explicit AssemblerMIPS32(bool use_far_branches = false)
      : Assembler(Asm_MIPS32) {
    // This mode is only needed and implemented for MIPS32 and ARM.
    assert(!use_far_branches);
    (void)use_far_branches;
  }
  ~AssemblerMIPS32() override {
    if (BuildDefs::asserts()) {
      for (const Label *Label : CfgNodeLabels) {
        Label->finalCheck();
      }
      for (const Label *Label : LocalLabels) {
        Label->finalCheck();
      }
    }
  }

  MIPS32Fixup *createMIPS32Fixup(const RelocOp Reloc, const Constant *RelOp);

  void trap();

  void nop();

  void emitRsRt(IValueT Opcode, const Operand *OpRs, const Operand *OpRt,
                const char *InsnName);

  void emitRtRsImm16(IValueT Opcode, const Operand *OpRt, const Operand *OpRs,
                     uint32_t Imm, const char *InsnName);

  void emitRtRsImm16Rel(IValueT Opcode, const Operand *OpRt,
                        const Operand *OpRs, const Operand *OpImm,
                        const RelocOp Reloc, const char *InsnName);

  void emitFtRsImm16(IValueT Opcode, const Operand *OpFt, const Operand *OpRs,
                     uint32_t Imm, const char *InsnName);

  void emitRdRtSa(IValueT Opcode, const Operand *OpRd, const Operand *OpRt,
                  uint32_t Sa, const char *InsnName);

  void emitRdRsRt(IValueT Opcode, const Operand *OpRd, const Operand *OpRs,
                  const Operand *OpRt, const char *InsnName);

  void emitCOP1Fcmp(IValueT Opcode, FPInstDataFormat Format,
                    const Operand *OpFs, const Operand *OpFt, IValueT CC,
                    const char *InsnName);

  void emitCOP1FmtFsFd(IValueT Opcode, FPInstDataFormat Format,
                       const Operand *OpFd, const Operand *OpFs,
                       const char *InsnName);

  void emitCOP1FmtFtFsFd(IValueT Opcode, FPInstDataFormat Format,
                         const Operand *OpFd, const Operand *OpFs,
                         const Operand *OpFt, const char *InsnName);

  void emitCOP1FmtRtFsFd(IValueT Opcode, FPInstDataFormat Format,
                         const Operand *OpFd, const Operand *OpFs,
                         const Operand *OpRt, const char *InsnName);

  void emitCOP1MovRtFs(IValueT Opcode, const Operand *OpRt, const Operand *OpFs,
                       const char *InsnName);

  void emitBr(const CondMIPS32::Cond Cond, const Operand *OpRs,
              const Operand *OpRt, IOffsetT Offset);

  void abs_d(const Operand *OpFd, const Operand *OpFs);

  void abs_s(const Operand *OpFd, const Operand *OpFs);

  void addi(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void add_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void add_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void addu(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void addiu(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void addiu(const Operand *OpRt, const Operand *OpRs, const Operand *OpImm,
             const RelocOp Reloc);

  void and_(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void andi(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void b(Label *TargetLabel);

  void c_eq_d(const Operand *OpFd, const Operand *OpFs);

  void c_eq_s(const Operand *OpFd, const Operand *OpFs);

  void c_ole_d(const Operand *OpFd, const Operand *OpFs);

  void c_ole_s(const Operand *OpFd, const Operand *OpFs);

  void c_olt_d(const Operand *OpFd, const Operand *OpFs);

  void c_olt_s(const Operand *OpFd, const Operand *OpFs);

  void c_ueq_d(const Operand *OpFd, const Operand *OpFs);

  void c_ueq_s(const Operand *OpFd, const Operand *OpFs);

  void c_ule_d(const Operand *OpFd, const Operand *OpFs);

  void c_ule_s(const Operand *OpFd, const Operand *OpFs);

  void c_ult_d(const Operand *OpFd, const Operand *OpFs);

  void c_ult_s(const Operand *OpFd, const Operand *OpFs);

  void c_un_d(const Operand *OpFd, const Operand *OpFs);

  void c_un_s(const Operand *OpFd, const Operand *OpFs);

  void clz(const Operand *OpRd, const Operand *OpRs);

  void cvt_d_l(const Operand *OpFd, const Operand *OpFs);

  void cvt_d_s(const Operand *OpFd, const Operand *OpFs);

  void cvt_d_w(const Operand *OpFd, const Operand *OpFs);

  void cvt_s_d(const Operand *OpFd, const Operand *OpFs);

  void cvt_s_l(const Operand *OpFd, const Operand *OpFs);

  void cvt_s_w(const Operand *OpFd, const Operand *OpFs);

  void div(const Operand *OpRs, const Operand *OpRt);

  void div_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void div_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void divu(const Operand *OpRs, const Operand *OpRt);

  void jal(const ConstantRelocatable *Target);

  void jalr(const Operand *OpRs, const Operand *OpRd);

  void lui(const Operand *OpRt, const Operand *OpImm, const RelocOp Reloc);

  void ldc1(const Operand *OpRt, const Operand *OpBase, const Operand *OpOff,
            const RelocOp Reloc);

  void ll(const Operand *OpRt, const Operand *OpBase, const uint32_t Offset);

  void lw(const Operand *OpRt, const Operand *OpBase, const uint32_t Offset);

  void lwc1(const Operand *OpRt, const Operand *OpBase, const Operand *OpOff,
            const RelocOp Reloc);

  void mfc1(const Operand *OpRt, const Operand *OpFs);

  void mfhi(const Operand *OpRd);

  void mflo(const Operand *OpRd);

  void mov_d(const Operand *OpFd, const Operand *OpFs);

  void mov_s(const Operand *OpFd, const Operand *OpFs);

  void move(const Operand *OpRd, const Operand *OpRs);

  void movf(const Operand *OpRd, const Operand *OpRs, const Operand *OpCc);

  void movn(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void movn_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void movn_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void movt(const Operand *OpRd, const Operand *OpRs, const Operand *OpCc);

  void movz(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void movz_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void movz_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void mtc1(const Operand *OpRt, const Operand *OpFs);

  void mthi(const Operand *OpRs);

  void mtlo(const Operand *OpRs);

  void mul(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void mul_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void mul_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void mult(const Operand *OpRs, const Operand *OpRt);

  void multu(const Operand *OpRs, const Operand *OpRt);

  void nor(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void or_(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void ori(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void ret(void);

  void sc(const Operand *OpRt, const Operand *OpBase, const uint32_t Offset);

  void sll(const Operand *OpRd, const Operand *OpRt, const uint32_t Sa);

  void sllv(const Operand *OpRd, const Operand *OpRt, const Operand *OpRs);

  void slt(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void slti(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void sltiu(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void sltu(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void sqrt_d(const Operand *OpFd, const Operand *OpFs);

  void sqrt_s(const Operand *OpFd, const Operand *OpFs);

  void sra(const Operand *OpRd, const Operand *OpRt, const uint32_t Sa);

  void srav(const Operand *OpRd, const Operand *OpRt, const Operand *OpRs);

  void srl(const Operand *OpRd, const Operand *OpRt, const uint32_t Sa);

  void srlv(const Operand *OpRd, const Operand *OpRt, const Operand *OpRs);

  void sub_d(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void sub_s(const Operand *OpFd, const Operand *OpFs, const Operand *OpFt);

  void subu(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void sdc1(const Operand *OpRt, const Operand *OpBase, const Operand *OpOff,
            const RelocOp Reloc);

  void sw(const Operand *OpRt, const Operand *OpBase, const uint32_t Offset);

  void swc1(const Operand *OpRt, const Operand *OpBase, const Operand *OpOff,
            const RelocOp Reloc);

  void sync();

  void teq(const Operand *OpRs, const Operand *OpRt, const uint32_t TrapCode);

  void trunc_l_d(const Operand *OpFd, const Operand *OpFs);

  void trunc_l_s(const Operand *OpFd, const Operand *OpFs);

  void trunc_w_d(const Operand *OpFd, const Operand *OpFs);

  void trunc_w_s(const Operand *OpFd, const Operand *OpFs);

  void xor_(const Operand *OpRd, const Operand *OpRs, const Operand *OpRt);

  void xori(const Operand *OpRt, const Operand *OpRs, const uint32_t Imm);

  void bcc(const CondMIPS32::Cond Cond, const Operand *OpRs,
           const Operand *OpRt, Label *TargetLabel);

  void bzc(const CondMIPS32::Cond Cond, const Operand *OpRs,
           Label *TargetLabel);

  void alignFunction() override {
    const SizeT Align = 1 << getBundleAlignLog2Bytes();
    SizeT BytesNeeded = Utils::OffsetToAlignment(Buffer.getPosition(), Align);
    constexpr SizeT InstSize = sizeof(IValueT);
    assert(BytesNeeded % InstMIPS32::InstSize == 0);
    while (BytesNeeded > 0) {
      trap();
      BytesNeeded -= InstSize;
    }
  }

  SizeT getBundleAlignLog2Bytes() const override { return 4; }

  const char *getAlignDirective() const override { return ".p2alignl"; }

  llvm::ArrayRef<uint8_t> getNonExecBundlePadding() const override;

  void padWithNop(intptr_t Padding) override;

  void bind(Label *label);

  void emitTextInst(const std::string &Text, SizeT InstSize);

  Ice::Label *getCfgNodeLabel(SizeT NodeNumber) override {
    assert(NodeNumber < CfgNodeLabels.size());
    return CfgNodeLabels[NodeNumber];
  }

  Label *getOrCreateCfgNodeLabel(SizeT NodeNumber) {
    return getOrCreateLabel(NodeNumber, CfgNodeLabels);
  }

  Label *getOrCreateLocalLabel(SizeT Number) {
    return getOrCreateLabel(Number, LocalLabels);
  }

  void bindLocalLabel(const InstMIPS32Label *InstL, SizeT Number) {
    if (BuildDefs::dump() && !getFlags().getDisableHybridAssembly()) {
      constexpr SizeT InstSize = 0;
      emitTextInst(InstL->getLabelName() + ":", InstSize);
    }
    Label *L = getOrCreateLocalLabel(Number);
    if (!getPreliminary())
      this->bind(L);
  }

  bool fixupIsPCRel(FixupKind Kind) const override {
    (void)Kind;
    return false;
  }

  static bool classof(const Assembler *Asm) {
    return Asm->getKind() == Asm_MIPS32;
  }

private:
  ENABLE_MAKE_UNIQUE;

  using LabelVector = std::vector<Label *>;
  LabelVector CfgNodeLabels;
  LabelVector LocalLabels;

  // Returns the offset encoded in the branch instruction Inst.
  static IOffsetT decodeBranchOffset(IValueT Inst);

  Label *getOrCreateLabel(SizeT Number, LabelVector &Labels);

  void bindCfgNodeLabel(const CfgNode *) override;

  void emitInst(IValueT Value) {
    AssemblerBuffer::EnsureCapacity _(&Buffer);
    Buffer.emit<IValueT>(Value);
  }
};

} // end of namespace MIPS32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEASSEMBLERMIPS32_H
