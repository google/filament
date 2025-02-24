//===- subzero/src/IceAssemblerARM32.h - Assembler for ARM32 ----*- C++ -*-===//
//
// Copyright (c) 2013, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
//
// Modified by the Subzero authors.
//
//===----------------------------------------------------------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Assembler class for ARM32.
///
/// Note: All references to ARM "section" documentation refers to the "ARM
/// Architecture Reference Manual, ARMv7-A and ARMv7-R edition". See:
/// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0406c
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEASSEMBLERARM32_H
#define SUBZERO_SRC_ICEASSEMBLERARM32_H

#include "IceAssembler.h"
#include "IceConditionCodesARM32.h"
#include "IceDefs.h"
#include "IceFixups.h"
#include "IceInstARM32.h"
#include "IceRegistersARM32.h"
#include "IceTargetLowering.h"

namespace Ice {
namespace ARM32 {

/// Handles encoding of bottom/top 16 bits of an address using movw/movt.
class MoveRelocatableFixup final : public AssemblerFixup {
  MoveRelocatableFixup &operator=(const MoveRelocatableFixup &) = delete;
  MoveRelocatableFixup(const MoveRelocatableFixup &) = default;

public:
  MoveRelocatableFixup() = default;
  size_t emit(GlobalContext *Ctx, const Assembler &Asm) const final;
  void emitOffset(Assembler *Asm) const;
};

/// Handles encoding of branch and link to global location.
class BlRelocatableFixup final : public AssemblerFixup {
  BlRelocatableFixup(const BlRelocatableFixup &) = delete;
  BlRelocatableFixup &operator=(const BlRelocatableFixup &) = delete;

public:
  BlRelocatableFixup() = default;
  size_t emit(GlobalContext *Ctx, const Assembler &Asm) const final;
  void emitOffset(Assembler *Asm) const;
};

class AssemblerARM32 : public Assembler {
  AssemblerARM32(const AssemblerARM32 &) = delete;
  AssemblerARM32 &operator=(const AssemblerARM32 &) = delete;

public:
  // Rotation values.
  enum RotationValue {
    kRotateNone, // Omitted
    kRotate8,    // ror #8
    kRotate16,   // ror #16
    kRotate24    // ror #24
  };

  // Encoding of the number of D registers in a list of D registers.
  enum DRegListSize {
    DRegListSize1 = 7,  // 0b0111
    DRegListSize2 = 10, // 0b1010
    DRegListSIze3 = 6,  // 0b0110
    DRegListSize4 = 2   // 0b0010
  };

  class TargetInfo {
    TargetInfo(const TargetInfo &) = delete;
    TargetInfo &operator=(const TargetInfo &) = delete;

  public:
    TargetInfo(bool HasFramePointer, RegNumT FrameOrStackReg)
        : HasFramePointer(HasFramePointer), FrameOrStackReg(FrameOrStackReg) {}
    explicit TargetInfo(const TargetLowering *Target)
        : HasFramePointer(Target->hasFramePointer()),
          FrameOrStackReg(Target->getFrameOrStackReg()) {}
    const bool HasFramePointer;
    const RegNumT FrameOrStackReg;
  };

  AssemblerARM32() : Assembler(Asm_ARM32) {}
  ~AssemblerARM32() override {
    if (BuildDefs::asserts()) {
      for (const Label *Label : CfgNodeLabels) {
        Label->finalCheck();
      }
      for (const Label *Label : LocalLabels) {
        Label->finalCheck();
      }
    }
  }

  MoveRelocatableFixup *createMoveFixup(bool IsMovW, const Constant *Value);

  BlRelocatableFixup *createBlFixup(const ConstantRelocatable *BlTarget);

  void alignFunction() override {
    const SizeT Align = 1 << getBundleAlignLog2Bytes();
    SizeT BytesNeeded = Utils::OffsetToAlignment(Buffer.getPosition(), Align);
    constexpr SizeT InstSize = sizeof(IValueT);
    assert(BytesNeeded % InstARM32::InstSize == 0);
    while (BytesNeeded > 0) {
      trap();
      BytesNeeded -= InstSize;
    }
  }

  SizeT getBundleAlignLog2Bytes() const override { return 4; }

  const char *getAlignDirective() const override { return ".p2alignl"; }

  llvm::ArrayRef<uint8_t> getNonExecBundlePadding() const override;

  void padWithNop(intptr_t Padding) override;

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

  void bindLocalLabel(const InstARM32Label *InstL, SizeT Number) {
    if (BuildDefs::dump() && !getFlags().getDisableHybridAssembly()) {
      constexpr SizeT InstSize = 0;
      emitTextInst(InstL->getLabelName() + ":", InstSize);
    }
    Label *L = getOrCreateLocalLabel(Number);
    if (!getPreliminary())
      this->bind(L);
  }

  bool fixupIsPCRel(FixupKind Kind) const override {
    if (Kind == llvm::ELF::R_ARM_MOVW_PREL_NC)
      return true;
    if (Kind == llvm::ELF::R_ARM_MOVT_PREL)
      return true;
    return false;
  }

  /// Accessors to keep track of the number of bytes generated inside
  /// InstARM32::emit() methods, when run inside of
  /// InstARM32::emitUsingTextFixup().
  void resetEmitTextSize() { EmitTextSize = 0; }
  void incEmitTextSize(size_t Amount) { EmitTextSize += Amount; }
  void decEmitTextSize(size_t Amount) { EmitTextSize -= Amount; }
  size_t getEmitTextSize() const { return EmitTextSize; }

  void bind(Label *label);

  // List of instructions implemented by integrated assembler.

  void adc(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void add(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void and_(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
            bool SetFlags, CondARM32::Cond Cond);

  void asr(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void b(Label *L, CondARM32::Cond Cond);

  void bkpt(uint16_t Imm16);

  void bic(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void bl(const ConstantRelocatable *Target);

  void blx(const Operand *Target);

  void bx(RegARM32::GPRRegister Rm, CondARM32::Cond Cond = CondARM32::AL);

  void clz(const Operand *OpRd, const Operand *OpSrc, CondARM32::Cond Cond);

  void cmn(const Operand *OpRn, const Operand *OpSrc1, CondARM32::Cond Cond);

  void cmp(const Operand *OpRn, const Operand *OpSrc1, CondARM32::Cond Cond);

  void dmb(IValueT Option); // Option is a 4-bit value.

  void eor(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void ldr(const Operand *OpRt, const Operand *OpAddress, CondARM32::Cond Cond,
           const TargetInfo &TInfo);

  void ldr(const Operand *OpRt, const Operand *OpAddress, CondARM32::Cond Cond,
           const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    ldr(OpRt, OpAddress, Cond, TInfo);
  }

  void ldrex(const Operand *OpRt, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void ldrex(const Operand *OpRt, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    ldrex(OpRt, OpAddress, Cond, TInfo);
  }

  void lsl(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void lsr(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void mov(const Operand *OpRd, const Operand *OpSrc, CondARM32::Cond Cond);

  void movw(const Operand *OpRd, const Operand *OpSrc, CondARM32::Cond Cond);

  void movt(const Operand *OpRd, const Operand *OpSrc, CondARM32::Cond Cond);

  void mla(const Operand *OpRd, const Operand *OpRn, const Operand *OpRm,
           const Operand *OpRa, CondARM32::Cond Cond);

  void mls(const Operand *OpRd, const Operand *OpRn, const Operand *OpRm,
           const Operand *OpRa, CondARM32::Cond Cond);

  void mul(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void mvn(const Operand *OpRd, const Operand *OpScc, CondARM32::Cond Cond);

  void nop();

  void orr(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void pop(const Variable *OpRt, CondARM32::Cond Cond);

  // Note: Registers is a bitset, where bit n corresponds to register Rn.
  void popList(const IValueT Registers, CondARM32::Cond Cond);

  void push(const Operand *OpRt, CondARM32::Cond Cond);

  // Note: Registers is a bitset, where bit n corresponds to register Rn.
  void pushList(const IValueT Registers, CondARM32::Cond Cond);

  void rbit(const Operand *OpRd, const Operand *OpRm, CondARM32::Cond Cond);

  void rev(const Operand *OpRd, const Operand *OpRm, CondARM32::Cond Cond);

  void rsb(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void rsc(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void sbc(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  void sdiv(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
            CondARM32::Cond Cond);

  void str(const Operand *OpRt, const Operand *OpAddress, CondARM32::Cond Cond,
           const TargetInfo &TInfo);

  void str(const Operand *OpRt, const Operand *OpAddress, CondARM32::Cond Cond,
           const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    str(OpRt, OpAddress, Cond, TInfo);
  }

  void strex(const Operand *OpRd, const Operand *OpRt, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void strex(const Operand *OpRd, const Operand *OpRt, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    strex(OpRd, OpRt, OpAddress, Cond, TInfo);
  }

  void sub(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
           bool SetFlags, CondARM32::Cond Cond);

  // Implements sxtb/sxth depending on type of OpSrc0.
  void sxt(const Operand *OpRd, const Operand *OpSrc0, CondARM32::Cond Cond);

  void trap();

  void tst(const Operand *OpRn, const Operand *OpSrc1, CondARM32::Cond Cond);

  void udiv(const Operand *OpRd, const Operand *OpRn, const Operand *OpSrc1,
            CondARM32::Cond Cond);

  void umull(const Operand *OpRdLo, const Operand *OpRdHi, const Operand *OpRn,
             const Operand *OpRm, CondARM32::Cond Cond);

  // Implements uxtb/uxth depending on type of OpSrc0.
  void uxt(const Operand *OpRd, const Operand *OpSrc0, CondARM32::Cond Cond);

  void vabss(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  void vabsd(const Operand *OpDd, const Operand *OpDm, CondARM32::Cond Cond);

  void vabsq(const Operand *OpQd, const Operand *OpQm);

  void vaddd(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  void vadds(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  // Integer vector add.
  void vaddqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  // Float vector add.
  void vaddqf(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vandq(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vbslq(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vceqqi(const Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  void vceqqs(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vcgeqi(const Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  void vcugeqi(const Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);

  void vcgeqs(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vcgtqi(const Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  void vcugtqi(const Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);

  void vcgtqs(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vcmpd(const Operand *OpDd, const Operand *OpDm, CondARM32::Cond cond);

  // Second argument of compare is zero (+0.0).
  void vcmpdz(const Operand *OpDd, CondARM32::Cond cond);

  void vcmps(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond cond);

  // Second argument of compare is zero (+0.0).
  void vcmpsz(const Operand *OpSd, CondARM32::Cond cond);

  void vcvtds(const Operand *OpDd, const Operand *OpSm, CondARM32::Cond Cond);

  // vcvt<c>.S32.F32
  void vcvtis(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  // vcvt<c>.S32.F64
  void vcvtid(const Operand *OpSd, const Operand *OpDm, CondARM32::Cond Cond);

  // vcvt<c>.F64.S32
  void vcvtdi(const Operand *OpDd, const Operand *OpSm, CondARM32::Cond Cond);

  // vcvt<c>.F64.U32
  void vcvtdu(const Operand *OpDd, const Operand *OpSm, CondARM32::Cond Cond);

  void vcvtsd(const Operand *OpSd, const Operand *OpDm, CondARM32::Cond Cond);

  // vcvt<c>.F32.S32
  void vcvtsi(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  // vcvt<c>.F32.U32
  void vcvtsu(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  // vcvt<c>.U32.F64
  void vcvtud(const Operand *OpSd, const Operand *OpDm, CondARM32::Cond Cond);

  // vcvt<c>.u32.f32
  void vcvtus(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  void vcvtqsi(const Operand *OpQd, const Operand *OpQm);

  void vcvtqsu(const Operand *OpQd, const Operand *OpQm);

  void vcvtqis(const Operand *OpQd, const Operand *OpQm);

  void vcvtqus(const Operand *OpQd, const Operand *OpQm);

  void vdivd(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  void vdivs(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  void veord(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm);

  void veorq(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);

  void vldrd(const Operand *OpDd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vldrd(const Operand *OpDd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vldrd(OpDd, OpAddress, Cond, TInfo);
  }

  void vldrs(const Operand *OpSd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vldrs(const Operand *OpSd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vldrs(OpSd, OpAddress, Cond, TInfo);
  }

  void vldrq(const Operand *OpQd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vldrq(const Operand *OpQd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vldrq(OpQd, OpAddress, Cond, TInfo);
  }

  // ElmtSize = #bits in vector element.
  void vld1qr(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
              const TargetInfo &TInfo);

  void vld1(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
            const TargetInfo &TInfo);

  void vld1qr(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
              const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vld1qr(ElmtSize, OpQd, OpRn, TInfo);
  }

  void vld1(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
            const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vld1(ElmtSize, OpQd, OpRn, TInfo);
  }

  // Qn[i] = Imm for all i in vector. Returns true iff Imm can be defined as an
  // Imm8 using AdvSIMDExpandImm().
  bool vmovqc(const Operand *OpQd, const ConstantInteger32 *Imm);

  // Dn = FpImm
  void vmovd(const Operand *OpDn, const OperandARM32FlexFpImm *OpFpImm,
             CondARM32::Cond Cond);

  // Dd = Dm
  void vmovdd(const Operand *OpDd, const Variable *OpDm, CondARM32::Cond Cond);

  // Dm = Rt:Rt2
  void vmovdrr(const Operand *OpDm, const Operand *OpRt, const Operand *OpRt2,
               CondARM32::Cond Cond);

  // Qd[Index] = Rt
  void vmovqir(const Operand *OpQd, uint32_t Index, const Operand *OpRt,
               CondARM32::Cond Cond);

  // Qd[Index] = Sm
  void vmovqis(const Operand *OpQd, uint32_t Indx, const Operand *OpSm,
               CondARM32::Cond Cond);

  // Rt = Qm[Index]
  void vmovrqi(const Operand *OpRt, const Operand *OpQd, uint32_t Index,
               CondARM32::Cond Cond);

  // Rt:Rt2 = Dm
  void vmovrrd(const Operand *OpRt, const Operand *OpRt2, const Operand *OpDm,
               CondARM32::Cond Cond);

  // Rt = Sn
  void vmovrs(const Operand *OpRt, const Operand *OpSn, CondARM32::Cond Cond);

  // Sn = FpImm
  void vmovs(const Operand *OpSn, const OperandARM32FlexFpImm *OpFpImm,
             CondARM32::Cond Cond);

  // Sd = Sm
  void vmovss(const Operand *OpSd, const Variable *OpSm, CondARM32::Cond Cond);

  // Sd = Qm[Index]
  void vmovsqi(const Operand *OpSd, const Operand *OpQm, uint32_t Index,
               CondARM32::Cond Cond);

  // Sn = Rt
  void vmovsr(const Operand *OpSn, const Operand *OpRt, CondARM32::Cond Cond);

  void vmlad(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  void vmlas(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  void vmlsd(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  void vmlss(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  // Uses APSR_nzcv as register
  void vmrsAPSR_nzcv(CondARM32::Cond Cond);

  void vmuld(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  // Integer vector multiply.
  void vmulqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
              const Operand *OpQm);

  // Integer vector multiply high.
  void vmulh(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
             const Operand *OpQm, bool Unsigned);

  // Integer vector multiply add pairwise.
  void vmlap(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
             const Operand *OpQm);

  // Vector element replication.
  void vdup(Type ElmtTy, const Operand *OpQd, const Operand *OpQn, IValueT Idx);

  // Vector interleave lower halves.
  void vzip(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
            const Operand *OpQm);

  // Float vector multiply.
  void vmulqf(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);

  void vmuls(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  void vmvnq(const Operand *OpQd, const Operand *OpQm);

  void vmovlq(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);
  void vmovhq(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);
  void vmovhlq(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);
  void vmovlhq(const Operand *OpQd, const Operand *OpQn, const Operand *OpQm);

  void vnegqs(const Operand *OpQd, const Operand *OpQm);

  void vnegqs(Type ElmtTy, const Operand *OpQd, const Operand *OpQm);

  void vorrq(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vpop(const Variable *OpBaseReg, SizeT NumConsecRegs,
            CondARM32::Cond Cond);

  void vpush(const Variable *OpBaseReg, SizeT NumConsecRegs,
             CondARM32::Cond Cond);

  void vshlqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  void vshlqu(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  void vshlqc(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const ConstantInteger32 *OpQn);

  void vshrqc(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const ConstantInteger32 *OpQn, InstARM32::FPSign Sign);

  void vsqrtd(const Operand *OpDd, const Operand *OpDm, CondARM32::Cond Cond);

  void vsqrts(const Operand *OpSd, const Operand *OpSm, CondARM32::Cond Cond);

  void vstrd(const Operand *OpDd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vstrd(const Operand *OpDd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vstrd(OpDd, OpAddress, Cond, TInfo);
  }

  void vstrs(const Operand *OpSd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vstrs(const Operand *OpSd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vstrs(OpSd, OpAddress, Cond, TInfo);
  }

  void vstrq(const Operand *OpQd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetInfo &TInfo);

  void vstrq(const Operand *OpQd, const Operand *OpAddress,
             CondARM32::Cond Cond, const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vstrq(OpQd, OpAddress, Cond, TInfo);
  }

  // ElmtSize = #bits in vector element.
  void vst1qr(size_t ElmtSize, const Operand *OpQd, const Operand *OpAddress,
              const TargetInfo &TInfo);

  void vst1qr(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
              const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vst1qr(ElmtSize, OpQd, OpRn, TInfo);
  }

  void vst1(size_t ElmtSize, const Operand *OpQd, const Operand *OpAddress,
            const TargetInfo &TInfo);

  void vst1(size_t ElmtSize, const Operand *OpQd, const Operand *OpRn,
            const TargetLowering *Lowering) {
    const TargetInfo TInfo(Lowering);
    vst1(ElmtSize, OpQd, OpRn, TInfo);
  }

  void vsubd(const Operand *OpDd, const Operand *OpDn, const Operand *OpDm,
             CondARM32::Cond Cond);

  // Integer vector subtract.
  void vsubqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
              const Operand *OpQn);

  // Integer vector saturating subtract.
  void vqsubqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);
  void vqsubqu(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);

  // Integer vector saturating add.
  void vqaddqi(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);
  void vqaddqu(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn);

  // Integer vector packing with optional saturation.
  void vqmovn2(Type ElmtTy, const Operand *OpQd, const Operand *OpQm,
               const Operand *OpQn, bool Unsigned, bool Saturating);

  // Float vector subtract
  void vsubqf(const Operand *OpQd, const Operand *OpQm, const Operand *OpQn);

  void vsubs(const Operand *OpSd, const Operand *OpSn, const Operand *OpSm,
             CondARM32::Cond Cond);

  static bool classof(const Assembler *Asm) {
    return Asm->getKind() == Asm_ARM32;
  }

  void emitTextInst(const std::string &Text, SizeT InstSize);

private:
  ENABLE_MAKE_UNIQUE;

  // A vector of pool-allocated x86 labels for CFG nodes.
  using LabelVector = std::vector<Label *>;
  LabelVector CfgNodeLabels;
  // A vector of pool-allocated x86 labels for Local labels.
  LabelVector LocalLabels;
  // Number of bytes emitted by InstARM32::emit() methods, when run inside
  // InstARM32::emitUsingTextFixup().
  size_t EmitTextSize = 0;

  // Load/store multiple addressing mode.
  enum BlockAddressMode {
    // bit encoding P U W
    DA = (0 | 0 | 0) << 21,   // decrement after
    IA = (0 | 4 | 0) << 21,   // increment after
    DB = (8 | 0 | 0) << 21,   // decrement before
    IB = (8 | 4 | 0) << 21,   // increment before
    DA_W = (0 | 0 | 1) << 21, // decrement after with writeback to base
    IA_W = (0 | 4 | 1) << 21, // increment after with writeback to base
    DB_W = (8 | 0 | 1) << 21, // decrement before with writeback to base
    IB_W = (8 | 4 | 1) << 21  // increment before with writeback to base
  };

  Label *getOrCreateLabel(SizeT Number, LabelVector &Labels);

  void bindCfgNodeLabel(const CfgNode *Node) override;

  // SIMD encoding for the vector ElmtTy.
  static IValueT encodeElmtType(Type ElmtTy);

  void emitInst(IValueT Value) {
    AssemblerBuffer::EnsureCapacity _(&Buffer);
    Buffer.emit<IValueT>(Value);
  }

  // List of possible checks to apply when calling emitType01() (below).
  enum EmitChecks { NoChecks, RdIsPcAndSetFlags };

  // Pattern cccctttoooosnnnnddddiiiiiiiiiiii where cccc=Cond, ttt=InstType,
  // s=SetFlags, oooo=Opcode, nnnn=Rn, dddd=Rd, iiiiiiiiiiii=imm12 (See ARM
  // section A5.2.3).
  void emitType01(CondARM32::Cond Cond, IValueT InstType, IValueT Opcode,
                  bool SetFlags, IValueT Rn, IValueT Rd, IValueT imm12,
                  EmitChecks RuleChecks, const char *InstName);

  // Converts appropriate representation on a data operation, and then calls
  // emitType01 above.
  void emitType01(CondARM32::Cond Cond, IValueT Opcode, const Operand *OpRd,
                  const Operand *OpRn, const Operand *OpSrc1, bool SetFlags,
                  EmitChecks RuleChecks, const char *InstName);

  // Same as above, but the value for Rd and Rn have already been converted
  // into instruction values.
  void emitType01(CondARM32::Cond Cond, IValueT Opcode, IValueT OpRd,
                  IValueT OpRn, const Operand *OpSrc1, bool SetFlags,
                  EmitChecks RuleChecks, const char *InstName);

  void emitType05(CondARM32::Cond Cond, int32_t Offset, bool Link);

  // Emit ccccoooaabalnnnnttttaaaaaaaaaaaa where cccc=Cond,
  // ooo=InstType, l=isLoad, b=isByte, and
  // aaa0a0aaaa0000aaaaaaaaaaaa=Address. Note that Address is assumed to be
  // defined by decodeAddress() in IceAssemblerARM32.cpp.
  void emitMemOp(CondARM32::Cond Cond, IValueT InstType, bool IsLoad,
                 bool IsByte, IValueT Rt, IValueT Address);

  // Emit ccccxxxxxxxxxxxxddddxxxxxxxxmmmm where cccc=Cond,
  // xxxxxxxxxxxx0000xxxxxxxx0000=Opcode, dddd=Rd, and mmmm=Rm.
  void emitRdRm(CondARM32::Cond Cond, IValueT Opcode, const Operand *OpRd,
                const Operand *OpRm, const char *InstName);

  // Emit ldr/ldrb/str/strb instruction with given address.
  void emitMemOp(CondARM32::Cond Cond, bool IsLoad, bool IsByte, IValueT Rt,
                 const Operand *OpAddress, const TargetInfo &TInfo,
                 const char *InstName);

  // Emit ldrh/ldrd/strh/strd instruction with given address using encoding 3.
  void emitMemOpEnc3(CondARM32::Cond Cond, IValueT Opcode, IValueT Rt,
                     const Operand *OpAddress, const TargetInfo &TInfo,
                     const char *InstName);

  // Emit cccc00011xxlnnnndddd11111001tttt where cccc=Cond, xx encodes type
  // size, l=IsLoad, nnnn=Rn (as defined by OpAddress), and tttt=Rt.
  void emitMemExOp(CondARM32::Cond, Type Ty, bool IsLoad, const Operand *OpRd,
                   IValueT Rt, const Operand *OpAddress,
                   const TargetInfo &TInfo, const char *InstName);

  // Pattern cccc100aaaalnnnnrrrrrrrrrrrrrrrr where cccc=Cond,
  // aaaa<<21=AddressMode, l=IsLoad, nnnn=BaseReg, and
  // rrrrrrrrrrrrrrrr is bitset of Registers.
  void emitMultiMemOp(CondARM32::Cond Cond, BlockAddressMode AddressMode,
                      bool IsLoad, IValueT BaseReg, IValueT Registers);

  // Pattern ccccxxxxxDxxxxxxddddxxxxiiiiiiii where cccc=Cond, ddddD=BaseReg,
  // iiiiiiii=NumConsecRegs, and xxxxx0xxxxxx0000xxxx00000000=Opcode.
  void emitVStackOp(CondARM32::Cond Cond, IValueT Opcode,
                    const Variable *OpBaseReg, SizeT NumConsecRegs);

  // Pattern cccc111xxDxxxxxxdddd101xxxMxmmmm where cccc=Cond, ddddD=Sd,
  // Mmmmm=Dm, and xx0xxxxxxdddd000xxx0x0000=Opcode.
  void emitVFPsd(CondARM32::Cond Cond, IValueT Opcode, IValueT Sd, IValueT Dm);

  // Pattern cccc111xxDxxxxxxdddd101xxxMxmmmm where cccc=Cond, Ddddd=Dd,
  // mmmmM=Sm, and xx0xxxxxxdddd000xxx0x0000=Opcode.
  void emitVFPds(CondARM32::Cond Cond, IValueT Opcode, IValueT Dd, IValueT Sm);

  // Pattern 111100000D00nnnnddddttttssaammmm | Opcode where Ddddd=Dd, nnnn=Rn,
  // mmmmm=Rm, tttt=NumDRegs, ElmtSize in {8, 16, 32, 64) and defines ss, and
  // aa=Align.
  void emitVMem1Op(IValueT Opcode, IValueT Dd, IValueT Rn, IValueT Rm,
                   DRegListSize NumDRegs, size_t ElmtSize, IValueT Align,
                   const char *InstName);

  // Pattern 111100000D00nnnnddddss00aaaammmm | Opcode where Ddddd=Dd, nnnn=Rn,
  // mmmmm=Rm, ElmtSize in {8, 16, 32) and defines ss, and aa=Align.
  void emitVMem1Op(IValueT Opcode, IValueT Dd, IValueT Rn, IValueT Rm,
                   size_t ElmtSize, IValueT Align, const char *InstName);

  // Pattern cccc011100x1dddd1111mmmm0001nnn where cccc=Cond,
  // x=Opcode, dddd=Rd, nnnn=Rn, mmmm=Rm.
  void emitDivOp(CondARM32::Cond Cond, IValueT Opcode, IValueT Rd, IValueT Rn,
                 IValueT Rm);

  // cccc1110iiiennnntttt1011Njj10000 where cccc=Cond, tttt=Rt, Ndddd=2*Qn=Dn,
  // iii=Opcode1, jj=Opcode2, Opcode1Opcode2 encodes Index and the
  // corresponding element size of the vector element, and e=IsExtract.
  void emitInsertExtractInt(CondARM32::Cond Cond, const Operand *OpQn,
                            uint32_t Index, const Operand *OpRt, bool IsExtract,
                            const char *InstName);

  // cccc11101D110000dddd101001M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  // Assigns Sd the value of Sm.
  void emitMoveSS(CondARM32::Cond Cond, IValueT Sd, IValueT Sm);

  // Pattern ccccxxxxxxxfnnnnddddssss1001mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, ssss=Rs, f=SetFlags and xxxxxxx=Opcode.
  void emitMulOp(CondARM32::Cond Cond, IValueT Opcode, IValueT Rd, IValueT Rn,
                 IValueT Rm, IValueT Rs, bool SetFlags);

  // Pattern cccc0001101s0000ddddxxxxxtt0mmmm where cccc=Cond, s=SetFlags,
  // dddd=Rd, mmmm=Rm, tt=Shift, and xxxxx is defined by OpSrc1. OpSrc1 defines
  // either xxxxx=Imm5, or xxxxx=ssss0 where ssss=Rs.
  void emitShift(const CondARM32::Cond Cond,
                 const OperandARM32::ShiftKind Shift, const Operand *OpRd,
                 const Operand *OpRm, const Operand *OpSrc1,
                 const bool SetFlags, const char *InstName);

  // Implements various forms of signed/unsigned extend value, using pattern
  // ccccxxxxxxxxnnnnddddrr000111mmmm where cccc=Cond, xxxxxxxx<<20=Opcode,
  // nnnn=Rn, dddd=Rd, rr=Rotation, and mmmm=Rm.
  void emitSignExtend(CondARM32::Cond, IValueT Opcode, const Operand *OpRd,
                      const Operand *OpSrc0, const char *InstName);

  // Implements various forms of vector (SIMD) operations.  Implements pattern
  // 111100100D00nnnndddn00F0NQM0mmmm where Dddd=Dd, Nnnn=Dn, Mmmm=Dm,
  // Q=UseQRegs, F=IsFloatTy, and Opcode is unioned into the pattern.
  void emitSIMDBase(IValueT Opcode, IValueT Dd, IValueT Dn, IValueT Dm,
                    bool UseQRegs, bool IsFloatTy);

  // Same as emitSIMDBase above, except ElmtShift=20 and ElmtSize is computed
  // from ElmtTy.
  void emitSIMD(IValueT Opcode, Type ElmtTy, IValueT Dd, IValueT Dn, IValueT Dm,
                bool UseQRegs);

  // Implements various integer forms of vector (SIMD) operations using Q
  // registers. Implements pattern 111100100D00nnn0ddd000F0N1M0mmm0 where
  // Dddd=Qd, Nnnn=Qn, Mmmm=Qm, F=IsFloatTy, and Opcode is unioned into the
  // pattern.
  void emitSIMDqqqBase(IValueT Opcode, const Operand *OpQd, const Operand *OpQn,
                       const Operand *OpQm, bool IsFloatTy,
                       const char *OpcodeName);

  // Same as emitSIMD above, except ElmtShift=20 and ElmtSize is computed from
  // ElmtTy.
  void emitSIMDqqq(IValueT Opcode, Type ElmtTy, const Operand *OpQd,
                   const Operand *OpQn, const Operand *OpQm,
                   const char *OpcodeName);

  // Implements various forms of vector (SIMD) shifts using Q registers.
  // Implements pattern 111100101Diiiiiidddd010101M1mmmm where Dddd=Qd, Mmmm=Qm,
  // iiiiii=Imm6, and Opcode is unioned into the pattern.
  void emitSIMDShiftqqc(IValueT Opcode, const Operand *OpQd,
                        const Operand *OpQm, const IValueT Imm6,
                        const char *OpcodeName);

  // Implements various forms of vector (SIMD) casts between (signed and
  // unsigned) integer and floating point types (f32). Implements pattern
  // 111100111D11ss11dddd011ooQM0mmmm where Dddd=Qd, Mmmm=Qm, 10=ss, op=00, 1=Q,
  // and Opcode is unioned into the pattern.
  void emitSIMDCvtqq(IValueT Opcode, const Operand *OpQd, const Operand *OpQm,
                     const char *CvtName);

  // Pattern cccctttxxxxnnnn0000iiiiiiiiiiii where cccc=Cond, nnnn=Rn,
  // ttt=Instruction type (derived from OpSrc1), iiiiiiiiiiii is derived from
  // OpSrc1, and xxxx=Opcode.
  void emitCompareOp(CondARM32::Cond Cond, IValueT Opcode, const Operand *OpRn,
                     const Operand *OpSrc1, const char *CmpName);

  void emitBranch(Label *L, CondARM32::Cond, bool Link);

  // Returns the offset encoded in the branch instruction Inst.
  static IOffsetT decodeBranchOffset(IValueT Inst);

  // Implements movw/movt, generating pattern ccccxxxxxxxsiiiiddddiiiiiiiiiiii
  // where cccc=Cond, xxxxxxx<<21=Opcode, dddd=Rd, s=SetFlags, and
  // iiiiiiiiiiiiiiii=Imm16.
  void emitMovwt(CondARM32::Cond Cond, bool IsMovw, const Operand *OpRd,
                 const Operand *OpSrc, const char *MovName);

  // Emit VFP instruction with 3 D registers.
  void emitVFPddd(CondARM32::Cond Cond, IValueT Opcode, const Operand *OpDd,
                  const Operand *OpDn, const Operand *OpDm,
                  const char *InstName);

  void emitVFPddd(CondARM32::Cond Cond, IValueT Opcode, IValueT Dd, IValueT Dn,
                  IValueT Dm);

  // Emit VFP instruction with 3 S registers.
  void emitVFPsss(CondARM32::Cond Cond, IValueT Opcode, IValueT Sd, IValueT Sn,
                  IValueT Sm);

  void emitVFPsss(CondARM32::Cond Cond, IValueT Opcode, const Operand *OpSd,
                  const Operand *OpSn, const Operand *OpSm,
                  const char *InstName);
};

} // end of namespace ARM32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEASSEMBLERARM32_H
