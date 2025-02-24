//===- subzero/src/IceAssemblerX8664.h - Assembler for x86-64 ---*- C++ -*-===//
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
/// \brief Declares the Assembler class for X86-64.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEASSEMBLERX8664_H
#define SUBZERO_SRC_ICEASSEMBLERX8664_H

#include "IceAssembler.h"
#include "IceConditionCodesX86.h"
#include "IceDefs.h"
#include "IceOperand.h"
#include "IceRegistersX8664.h"
#include "IceTypes.h"
#include "IceUtils.h"

namespace Ice {
namespace X8664 {

using BrCond = CondX86::BrCond;
using CmppsCond = CondX86::CmppsCond;
using RegisterSet = ::Ice::RegX8664;
using GPRRegister = RegisterSet::GPRRegister;
using ByteRegister = RegisterSet::ByteRegister;
using XmmRegister = RegisterSet::XmmRegister;

class X86OperandMem;
class TargetX8664;

constexpr FixupKind FK_PcRel = llvm::ELF::R_X86_64_PC32;
constexpr FixupKind FK_Abs = llvm::ELF::R_X86_64_32S;
constexpr FixupKind FK_Gotoff = llvm::ELF::R_X86_64_GOTOFF64;
constexpr FixupKind FK_GotPC = llvm::ELF::R_X86_64_GOTPC32;

enum ScaleFactor { TIMES_1 = 0, TIMES_2 = 1, TIMES_4 = 2, TIMES_8 = 3 };

class AsmOperand {
public:
  enum RexBits {
    RexNone = 0x00,
    RexBase = 0x40,
    RexW = RexBase | (1 << 3),
    RexR = RexBase | (1 << 2),
    RexX = RexBase | (1 << 1),
    RexB = RexBase | (1 << 0),
  };

protected:
  // Needed by subclass AsmAddress.
  AsmOperand() = default;

public:
  AsmOperand(const AsmOperand &) = default;
  AsmOperand(AsmOperand &&) = default;
  AsmOperand &operator=(const AsmOperand &) = default;
  AsmOperand &operator=(AsmOperand &&) = default;

  uint8_t mod() const { return (encoding_at(0) >> 6) & 3; }

  uint8_t rexX() const { return (rex_ & RexX) != RexX ? RexNone : RexX; }
  uint8_t rexB() const { return (rex_ & RexB) != RexB ? RexNone : RexB; }

  GPRRegister rm() const {
    return static_cast<GPRRegister>((rexB() != 0 ? 0x08 : 0) |
                                    (encoding_at(0) & 7));
  }

  ScaleFactor scale() const {
    return static_cast<ScaleFactor>((encoding_at(1) >> 6) & 3);
  }

  GPRRegister index() const {
    return static_cast<GPRRegister>((rexX() != 0 ? 0x08 : 0) |
                                    ((encoding_at(1) >> 3) & 7));
  }

  GPRRegister base() const {
    return static_cast<GPRRegister>((rexB() != 0 ? 0x08 : 0) |
                                    (encoding_at(1) & 7));
  }

  int8_t disp8() const {
    assert(length_ >= 2);
    return static_cast<int8_t>(encoding_[length_ - 1]);
  }

  AssemblerFixup *fixup() const { return fixup_; }

protected:
  void SetModRM(int mod, GPRRegister rm) {
    assert((mod & ~3) == 0);
    encoding_[0] = (mod << 6) | (rm & 0x07);
    rex_ = (rm & 0x08) ? RexB : RexNone;
    length_ = 1;
  }

  void SetSIB(ScaleFactor scale, GPRRegister index, GPRRegister base) {
    assert(length_ == 1);
    assert((scale & ~3) == 0);
    encoding_[1] = (scale << 6) | ((index & 0x07) << 3) | (base & 0x07);
    rex_ = ((base & 0x08) ? RexB : RexNone) | ((index & 0x08) ? RexX : RexNone);
    length_ = 2;
  }

  void SetDisp8(int8_t disp) {
    assert(length_ == 1 || length_ == 2);
    encoding_[length_++] = static_cast<uint8_t>(disp);
  }

  void SetDisp32(int32_t disp) {
    assert(length_ == 1 || length_ == 2);
    intptr_t disp_size = sizeof(disp);
    memmove(&encoding_[length_], &disp, disp_size);
    length_ += disp_size;
  }

  void SetFixup(AssemblerFixup *fixup) { fixup_ = fixup; }

private:
  AssemblerFixup *fixup_ = nullptr;
  uint8_t rex_ = 0;
  uint8_t encoding_[6];
  uint8_t length_ = 0;

  explicit AsmOperand(GPRRegister reg) : fixup_(nullptr) { SetModRM(3, reg); }

  /// Get the operand encoding byte at the given index.
  uint8_t encoding_at(intptr_t index) const {
    assert(index >= 0 && index < length_);
    return encoding_[index];
  }

  /// Returns whether or not this operand is really the given register in
  /// disguise. Used from the assembler to generate better encodings.
  bool IsRegister(GPRRegister reg) const {
    return ((encoding_[0] & 0xF8) == 0xC0) // Addressing mode is register only.
           && (rm() == reg);               // Register codes match.
  }

  friend class AssemblerX8664;
};

class AsmAddress : public AsmOperand {
  AsmAddress() = default;

public:
  AsmAddress(const Variable *Var, const TargetX8664 *Target);
  AsmAddress(const X86OperandMem *Mem, Ice::Assembler *Asm,
             const Ice::TargetLowering *Target);

  // Address into the constant pool.
  AsmAddress(const Constant *Imm, Ice::Assembler *Asm) {
    // TODO(jpp): ???
    AssemblerFixup *Fixup = Asm->createFixup(FK_Abs, Imm);
    const RelocOffsetT Offset = 4;
    SetRipRelative(Offset, Fixup);
  }

private:
  AsmAddress(const AsmAddress &) = default;
  AsmAddress(AsmAddress &&) = default;
  AsmAddress &operator=(const AsmAddress &) = default;
  AsmAddress &operator=(AsmAddress &&) = default;

  void SetBase(GPRRegister Base, int32_t Disp, AssemblerFixup *Fixup) {
    if (Fixup == nullptr && Disp == 0 &&
        (Base & 7) != RegX8664::Encoded_Reg_rbp) {
      SetModRM(0, Base);
      if ((Base & 7) == RegX8664::Encoded_Reg_rsp)
        SetSIB(TIMES_1, RegX8664::Encoded_Reg_rsp, Base);
    } else if (Fixup == nullptr && Utils::IsInt(8, Disp)) {
      SetModRM(1, Base);
      if ((Base & 7) == RegX8664::Encoded_Reg_rsp)
        SetSIB(TIMES_1, RegX8664::Encoded_Reg_rsp, Base);
      SetDisp8(Disp);
    } else {
      SetModRM(2, Base);
      if ((Base & 7) == RegX8664::Encoded_Reg_rsp)
        SetSIB(TIMES_1, RegX8664::Encoded_Reg_rsp, Base);
      SetDisp32(Disp);
      if (Fixup)
        SetFixup(Fixup);
    }
  }

  void SetIndex(GPRRegister Index, ScaleFactor Scale, int32_t Disp,
                AssemblerFixup *Fixup) {
    assert(Index != RegX8664::Encoded_Reg_rsp); // Illegal addressing mode.
    SetModRM(0, RegX8664::Encoded_Reg_rsp);
    SetSIB(Scale, Index, RegX8664::Encoded_Reg_rbp);
    SetDisp32(Disp);
    if (Fixup)
      SetFixup(Fixup);
  }

  void SetBaseIndex(GPRRegister Base, GPRRegister Index, ScaleFactor Scale,
                    int32_t Disp, AssemblerFixup *Fixup) {
    assert(Index != RegX8664::Encoded_Reg_rsp); // Illegal addressing mode.
    if (Fixup == nullptr && Disp == 0 &&
        (Base & 7) != RegX8664::Encoded_Reg_rbp) {
      SetModRM(0, RegX8664::Encoded_Reg_rsp);
      SetSIB(Scale, Index, Base);
    } else if (Fixup == nullptr && Utils::IsInt(8, Disp)) {
      SetModRM(1, RegX8664::Encoded_Reg_rsp);
      SetSIB(Scale, Index, Base);
      SetDisp8(Disp);
    } else {
      SetModRM(2, RegX8664::Encoded_Reg_rsp);
      SetSIB(Scale, Index, Base);
      SetDisp32(Disp);
      if (Fixup)
        SetFixup(Fixup);
    }
  }

  /// Generate a RIP-relative address expression on x86-64.
  void SetRipRelative(RelocOffsetT Offset, AssemblerFixup *Fixup) {
    assert(Fixup != nullptr);
    assert(Fixup->kind() == FK_PcRel);

    SetModRM(0x0, RegX8664::Encoded_Reg_rbp);

    // Use the Offset in the displacement for now. If we decide to process
    // fixups later, we'll need to patch up the emitted displacement.
    SetDisp32(Offset);
    if (Fixup)
      SetFixup(Fixup);
  }

  /// Generate an absolute address.
  void SetAbsolute(RelocOffsetT Addr) {
    SetModRM(0x0, RegX8664::Encoded_Reg_rsp);
    static constexpr ScaleFactor NoScale = TIMES_1;
    SetSIB(NoScale, RegX8664::Encoded_Reg_rsp, RegX8664::Encoded_Reg_rbp);
    SetDisp32(Addr);
  }
};

class AssemblerX8664 : public ::Ice::Assembler {
  AssemblerX8664(const AssemblerX8664 &) = delete;
  AssemblerX8664 &operator=(const AssemblerX8664 &) = delete;

protected:
  explicit AssemblerX8664() : Assembler(Asm_X8664) {}

public:
  static constexpr int MAX_NOP_SIZE = 8;

  static bool classof(const Assembler *Asm) {
    return Asm->getKind() == Asm_X8664;
  }

  class Immediate {
    Immediate(const Immediate &) = delete;
    Immediate &operator=(const Immediate &) = delete;

  public:
    explicit Immediate(int32_t value) : value_(value) {}

    explicit Immediate(AssemblerFixup *fixup) : fixup_(fixup) {}

    int32_t value() const { return value_; }
    AssemblerFixup *fixup() const { return fixup_; }

    bool is_int8() const {
      // We currently only allow 32-bit fixups, and they usually have value = 0,
      // so if fixup_ != nullptr, it shouldn't be classified as int8/16.
      return fixup_ == nullptr && Utils::IsInt(8, value_);
    }
    bool is_uint8() const {
      return fixup_ == nullptr && Utils::IsUint(8, value_);
    }
    bool is_uint16() const {
      return fixup_ == nullptr && Utils::IsUint(16, value_);
    }

  private:
    const int32_t value_ = 0;
    AssemblerFixup *fixup_ = nullptr;
  };

  /// X86 allows near and far jumps.
  class Label final : public Ice::Label {
    Label(const Label &) = delete;
    Label &operator=(const Label &) = delete;

  public:
    Label() = default;
    ~Label() = default;

    void finalCheck() const override {
      Ice::Label::finalCheck();
      assert(!hasNear());
    }

    /// Returns the position of an earlier branch instruction which assumes that
    /// this label is "near", and bumps iterator to the next near position.
    intptr_t getNearPosition() {
      assert(hasNear());
      intptr_t Pos = UnresolvedNearPositions.back();
      UnresolvedNearPositions.pop_back();
      return Pos;
    }

    bool hasNear() const { return !UnresolvedNearPositions.empty(); }
    bool isUnused() const override {
      return Ice::Label::isUnused() && !hasNear();
    }

  private:
    friend class AssemblerX8664;

    void nearLinkTo(const Assembler &Asm, intptr_t position) {
      if (Asm.getPreliminary())
        return;
      assert(!isBound());
      UnresolvedNearPositions.push_back(position);
    }

    llvm::SmallVector<intptr_t, 20> UnresolvedNearPositions;
  };

public:
  ~AssemblerX8664() override;

  static const bool kNearJump = true;
  static const bool kFarJump = false;

  void alignFunction() override;

  SizeT getBundleAlignLog2Bytes() const override { return 5; }

  const char *getAlignDirective() const override { return ".p2align"; }

  llvm::ArrayRef<uint8_t> getNonExecBundlePadding() const override {
    static const uint8_t Padding[] = {0xF4};
    return llvm::ArrayRef<uint8_t>(Padding, 1);
  }

  void padWithNop(intptr_t Padding) override {
    while (Padding > MAX_NOP_SIZE) {
      nop(MAX_NOP_SIZE);
      Padding -= MAX_NOP_SIZE;
    }
    if (Padding)
      nop(Padding);
  }

  Ice::Label *getCfgNodeLabel(SizeT NodeNumber) override;
  void bindCfgNodeLabel(const CfgNode *Node) override;
  Label *getOrCreateCfgNodeLabel(SizeT Number);
  Label *getOrCreateLocalLabel(SizeT Number);
  void bindLocalLabel(SizeT Number);

  bool fixupIsPCRel(FixupKind Kind) const override {
    // Currently assuming this is the only PC-rel relocation type used.
    return Kind == FK_PcRel;
  }

  // Operations to emit GPR instructions (and dispatch on operand type).
  using TypedEmitGPR = void (AssemblerX8664::*)(Type, GPRRegister);
  using TypedEmitAddr = void (AssemblerX8664::*)(Type, const AsmAddress &);
  struct GPREmitterOneOp {
    TypedEmitGPR Reg;
    TypedEmitAddr Addr;
  };

  using TypedEmitGPRGPR = void (AssemblerX8664::*)(Type, GPRRegister,
                                                   GPRRegister);
  using TypedEmitGPRAddr = void (AssemblerX8664::*)(Type, GPRRegister,
                                                    const AsmAddress &);
  using TypedEmitGPRImm = void (AssemblerX8664::*)(Type, GPRRegister,
                                                   const Immediate &);
  struct GPREmitterRegOp {
    TypedEmitGPRGPR GPRGPR;
    TypedEmitGPRAddr GPRAddr;
    TypedEmitGPRImm GPRImm;
  };

  struct GPREmitterShiftOp {
    // Technically, Addr/GPR and Addr/Imm are also allowed, but */Addr are
    // not. In practice, we always normalize the Dest to a Register first.
    TypedEmitGPRGPR GPRGPR;
    TypedEmitGPRImm GPRImm;
  };

  using TypedEmitGPRGPRImm = void (AssemblerX8664::*)(Type, GPRRegister,
                                                      GPRRegister,
                                                      const Immediate &);
  struct GPREmitterShiftD {
    // Technically AddrGPR and AddrGPRImm are also allowed, but in practice we
    // always normalize Dest to a Register first.
    TypedEmitGPRGPR GPRGPR;
    TypedEmitGPRGPRImm GPRGPRImm;
  };

  using TypedEmitAddrGPR = void (AssemblerX8664::*)(Type, const AsmAddress &,
                                                    GPRRegister);
  using TypedEmitAddrImm = void (AssemblerX8664::*)(Type, const AsmAddress &,
                                                    const Immediate &);
  struct GPREmitterAddrOp {
    TypedEmitAddrGPR AddrGPR;
    TypedEmitAddrImm AddrImm;
  };

  // Operations to emit XMM instructions (and dispatch on operand type).
  using TypedEmitXmmXmm = void (AssemblerX8664::*)(Type, XmmRegister,
                                                   XmmRegister);
  using TypedEmitXmmAddr = void (AssemblerX8664::*)(Type, XmmRegister,
                                                    const AsmAddress &);
  struct XmmEmitterRegOp {
    TypedEmitXmmXmm XmmXmm;
    TypedEmitXmmAddr XmmAddr;
  };

  using EmitXmmXmm = void (AssemblerX8664::*)(XmmRegister, XmmRegister);
  using EmitXmmAddr = void (AssemblerX8664::*)(XmmRegister, const AsmAddress &);
  using EmitAddrXmm = void (AssemblerX8664::*)(const AsmAddress &, XmmRegister);
  struct XmmEmitterMovOps {
    EmitXmmXmm XmmXmm;
    EmitXmmAddr XmmAddr;
    EmitAddrXmm AddrXmm;
  };

  using TypedEmitXmmImm = void (AssemblerX8664::*)(Type, XmmRegister,
                                                   const Immediate &);

  struct XmmEmitterShiftOp {
    TypedEmitXmmXmm XmmXmm;
    TypedEmitXmmAddr XmmAddr;
    TypedEmitXmmImm XmmImm;
  };

  // Cross Xmm/GPR cast instructions.
  template <typename DReg_t, typename SReg_t> struct CastEmitterRegOp {
    using TypedEmitRegs = void (AssemblerX8664::*)(Type, DReg_t, Type, SReg_t);
    using TypedEmitAddr = void (AssemblerX8664::*)(Type, DReg_t, Type,
                                                   const AsmAddress &);

    TypedEmitRegs RegReg;
    TypedEmitAddr RegAddr;
  };

  // Three operand (potentially) cross Xmm/GPR instructions. The last operand
  // must be an immediate.
  template <typename DReg_t, typename SReg_t> struct ThreeOpImmEmitter {
    using TypedEmitRegRegImm = void (AssemblerX8664::*)(Type, DReg_t, SReg_t,
                                                        const Immediate &);
    using TypedEmitRegAddrImm = void (AssemblerX8664::*)(Type, DReg_t,
                                                         const AsmAddress &,
                                                         const Immediate &);

    TypedEmitRegRegImm RegRegImm;
    TypedEmitRegAddrImm RegAddrImm;
  };

  /*
   * Emit Machine Instructions.
   */
  void call(GPRRegister reg);
  void call(const AsmAddress &address);
  void call(const ConstantRelocatable *label); // not testable.
  void call(const Immediate &abs_address);

  static const intptr_t kCallExternalLabelSize = 5;

  void pushl(GPRRegister reg);
  void pushl(const Immediate &Imm);
  void pushl(const ConstantRelocatable *Label);

  void popl(GPRRegister reg);
  void popl(const AsmAddress &address);

  void setcc(BrCond condition, ByteRegister dst);
  void setcc(BrCond condition, const AsmAddress &address);

  void mov(Type Ty, GPRRegister dst, const Immediate &src);
  void mov(Type Ty, GPRRegister dst, GPRRegister src);
  void mov(Type Ty, GPRRegister dst, const AsmAddress &src);
  void mov(Type Ty, const AsmAddress &dst, GPRRegister src);
  void mov(Type Ty, const AsmAddress &dst, const Immediate &imm);

  void movabs(const GPRRegister Dst, uint64_t Imm64);

  void movzx(Type Ty, GPRRegister dst, GPRRegister src);
  void movzx(Type Ty, GPRRegister dst, const AsmAddress &src);
  void movsx(Type Ty, GPRRegister dst, GPRRegister src);
  void movsx(Type Ty, GPRRegister dst, const AsmAddress &src);

  void lea(Type Ty, GPRRegister dst, const AsmAddress &src);

  void cmov(Type Ty, BrCond cond, GPRRegister dst, GPRRegister src);
  void cmov(Type Ty, BrCond cond, GPRRegister dst, const AsmAddress &src);

  void rep_movsb();

  void movss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void movss(Type Ty, const AsmAddress &dst, XmmRegister src);
  void movss(Type Ty, XmmRegister dst, XmmRegister src);

  void movd(Type SrcTy, XmmRegister dst, GPRRegister src);
  void movd(Type SrcTy, XmmRegister dst, const AsmAddress &src);
  void movd(Type DestTy, GPRRegister dst, XmmRegister src);
  void movd(Type DestTy, const AsmAddress &dst, XmmRegister src);

  void movq(XmmRegister dst, XmmRegister src);
  void movq(const AsmAddress &dst, XmmRegister src);
  void movq(XmmRegister dst, const AsmAddress &src);

  void addss(Type Ty, XmmRegister dst, XmmRegister src);
  void addss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void subss(Type Ty, XmmRegister dst, XmmRegister src);
  void subss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void mulss(Type Ty, XmmRegister dst, XmmRegister src);
  void mulss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void divss(Type Ty, XmmRegister dst, XmmRegister src);
  void divss(Type Ty, XmmRegister dst, const AsmAddress &src);

  void movaps(XmmRegister dst, XmmRegister src);

  void movups(XmmRegister dst, XmmRegister src);
  void movups(XmmRegister dst, const AsmAddress &src);
  void movups(const AsmAddress &dst, XmmRegister src);

  void padd(Type Ty, XmmRegister dst, XmmRegister src);
  void padd(Type Ty, XmmRegister dst, const AsmAddress &src);
  void padds(Type Ty, XmmRegister dst, XmmRegister src);
  void padds(Type Ty, XmmRegister dst, const AsmAddress &src);
  void paddus(Type Ty, XmmRegister dst, XmmRegister src);
  void paddus(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pand(Type Ty, XmmRegister dst, XmmRegister src);
  void pand(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pandn(Type Ty, XmmRegister dst, XmmRegister src);
  void pandn(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pmull(Type Ty, XmmRegister dst, XmmRegister src);
  void pmull(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pmulhw(Type Ty, XmmRegister dst, XmmRegister src);
  void pmulhw(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pmulhuw(Type Ty, XmmRegister dst, XmmRegister src);
  void pmulhuw(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pmaddwd(Type Ty, XmmRegister dst, XmmRegister src);
  void pmaddwd(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pmuludq(Type Ty, XmmRegister dst, XmmRegister src);
  void pmuludq(Type Ty, XmmRegister dst, const AsmAddress &src);
  void por(Type Ty, XmmRegister dst, XmmRegister src);
  void por(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psub(Type Ty, XmmRegister dst, XmmRegister src);
  void psub(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psubs(Type Ty, XmmRegister dst, XmmRegister src);
  void psubs(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psubus(Type Ty, XmmRegister dst, XmmRegister src);
  void psubus(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pxor(Type Ty, XmmRegister dst, XmmRegister src);
  void pxor(Type Ty, XmmRegister dst, const AsmAddress &src);

  void psll(Type Ty, XmmRegister dst, XmmRegister src);
  void psll(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psll(Type Ty, XmmRegister dst, const Immediate &src);

  void psra(Type Ty, XmmRegister dst, XmmRegister src);
  void psra(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psra(Type Ty, XmmRegister dst, const Immediate &src);
  void psrl(Type Ty, XmmRegister dst, XmmRegister src);
  void psrl(Type Ty, XmmRegister dst, const AsmAddress &src);
  void psrl(Type Ty, XmmRegister dst, const Immediate &src);

  void addps(Type Ty, XmmRegister dst, XmmRegister src);
  void addps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void subps(Type Ty, XmmRegister dst, XmmRegister src);
  void subps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void divps(Type Ty, XmmRegister dst, XmmRegister src);
  void divps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void mulps(Type Ty, XmmRegister dst, XmmRegister src);
  void mulps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void minps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void minps(Type Ty, XmmRegister dst, XmmRegister src);
  void minss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void minss(Type Ty, XmmRegister dst, XmmRegister src);
  void maxps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void maxps(Type Ty, XmmRegister dst, XmmRegister src);
  void maxss(Type Ty, XmmRegister dst, const AsmAddress &src);
  void maxss(Type Ty, XmmRegister dst, XmmRegister src);
  void andnps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void andnps(Type Ty, XmmRegister dst, XmmRegister src);
  void andps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void andps(Type Ty, XmmRegister dst, XmmRegister src);
  void orps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void orps(Type Ty, XmmRegister dst, XmmRegister src);

  void blendvps(Type Ty, XmmRegister dst, XmmRegister src);
  void blendvps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pblendvb(Type Ty, XmmRegister dst, XmmRegister src);
  void pblendvb(Type Ty, XmmRegister dst, const AsmAddress &src);

  void cmpps(Type Ty, XmmRegister dst, XmmRegister src, CmppsCond CmpCondition);
  void cmpps(Type Ty, XmmRegister dst, const AsmAddress &src,
             CmppsCond CmpCondition);

  void sqrtps(XmmRegister dst);
  void rsqrtps(XmmRegister dst);
  void reciprocalps(XmmRegister dst);

  void movhlps(XmmRegister dst, XmmRegister src);
  void movlhps(XmmRegister dst, XmmRegister src);
  void unpcklps(XmmRegister dst, XmmRegister src);
  void unpckhps(XmmRegister dst, XmmRegister src);
  void unpcklpd(XmmRegister dst, XmmRegister src);
  void unpckhpd(XmmRegister dst, XmmRegister src);

  void set1ps(XmmRegister dst, GPRRegister tmp, const Immediate &imm);

  void sqrtpd(XmmRegister dst);

  void pshufb(Type Ty, XmmRegister dst, XmmRegister src);
  void pshufb(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pshufd(Type Ty, XmmRegister dst, XmmRegister src, const Immediate &mask);
  void pshufd(Type Ty, XmmRegister dst, const AsmAddress &src,
              const Immediate &mask);
  void punpckl(Type Ty, XmmRegister Dst, XmmRegister Src);
  void punpckl(Type Ty, XmmRegister Dst, const AsmAddress &Src);
  void punpckh(Type Ty, XmmRegister Dst, XmmRegister Src);
  void punpckh(Type Ty, XmmRegister Dst, const AsmAddress &Src);
  void packss(Type Ty, XmmRegister Dst, XmmRegister Src);
  void packss(Type Ty, XmmRegister Dst, const AsmAddress &Src);
  void packus(Type Ty, XmmRegister Dst, XmmRegister Src);
  void packus(Type Ty, XmmRegister Dst, const AsmAddress &Src);
  void shufps(Type Ty, XmmRegister dst, XmmRegister src, const Immediate &mask);
  void shufps(Type Ty, XmmRegister dst, const AsmAddress &src,
              const Immediate &mask);

  void cvtdq2ps(Type, XmmRegister dst, XmmRegister src);
  void cvtdq2ps(Type, XmmRegister dst, const AsmAddress &src);

  void cvttps2dq(Type, XmmRegister dst, XmmRegister src);
  void cvttps2dq(Type, XmmRegister dst, const AsmAddress &src);

  void cvtps2dq(Type, XmmRegister dst, XmmRegister src);
  void cvtps2dq(Type, XmmRegister dst, const AsmAddress &src);

  void cvtsi2ss(Type DestTy, XmmRegister dst, Type SrcTy, GPRRegister src);
  void cvtsi2ss(Type DestTy, XmmRegister dst, Type SrcTy,
                const AsmAddress &src);

  void cvtfloat2float(Type SrcTy, XmmRegister dst, XmmRegister src);
  void cvtfloat2float(Type SrcTy, XmmRegister dst, const AsmAddress &src);

  void cvttss2si(Type DestTy, GPRRegister dst, Type SrcTy, XmmRegister src);
  void cvttss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                 const AsmAddress &src);

  void cvtss2si(Type DestTy, GPRRegister dst, Type SrcTy, XmmRegister src);
  void cvtss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                const AsmAddress &src);

  void ucomiss(Type Ty, XmmRegister a, XmmRegister b);
  void ucomiss(Type Ty, XmmRegister a, const AsmAddress &b);

  void movmsk(Type Ty, GPRRegister dst, XmmRegister src);

  void sqrt(Type Ty, XmmRegister dst, const AsmAddress &src);
  void sqrt(Type Ty, XmmRegister dst, XmmRegister src);

  void xorps(Type Ty, XmmRegister dst, const AsmAddress &src);
  void xorps(Type Ty, XmmRegister dst, XmmRegister src);

  void insertps(Type Ty, XmmRegister dst, XmmRegister src,
                const Immediate &imm);
  void insertps(Type Ty, XmmRegister dst, const AsmAddress &src,
                const Immediate &imm);

  void pinsr(Type Ty, XmmRegister dst, GPRRegister src, const Immediate &imm);
  void pinsr(Type Ty, XmmRegister dst, const AsmAddress &src,
             const Immediate &imm);

  void pextr(Type Ty, GPRRegister dst, XmmRegister src, const Immediate &imm);

  void pmovsxdq(XmmRegister dst, XmmRegister src);

  void pcmpeq(Type Ty, XmmRegister dst, XmmRegister src);
  void pcmpeq(Type Ty, XmmRegister dst, const AsmAddress &src);
  void pcmpgt(Type Ty, XmmRegister dst, XmmRegister src);
  void pcmpgt(Type Ty, XmmRegister dst, const AsmAddress &src);

  enum RoundingMode {
    kRoundToNearest = 0x0,
    kRoundDown = 0x1,
    kRoundUp = 0x2,
    kRoundToZero = 0x3
  };
  void round(Type Ty, XmmRegister dst, XmmRegister src, const Immediate &mode);
  void round(Type Ty, XmmRegister dst, const AsmAddress &src,
             const Immediate &mode);

  void cmp(Type Ty, GPRRegister reg0, GPRRegister reg1);
  void cmp(Type Ty, GPRRegister reg, const AsmAddress &address);
  void cmp(Type Ty, GPRRegister reg, const Immediate &imm);
  void cmp(Type Ty, const AsmAddress &address, GPRRegister reg);
  void cmp(Type Ty, const AsmAddress &address, const Immediate &imm);

  void test(Type Ty, GPRRegister reg0, GPRRegister reg1);
  void test(Type Ty, GPRRegister reg, const Immediate &imm);
  void test(Type Ty, const AsmAddress &address, GPRRegister reg);
  void test(Type Ty, const AsmAddress &address, const Immediate &imm);

  void And(Type Ty, GPRRegister dst, GPRRegister src);
  void And(Type Ty, GPRRegister dst, const AsmAddress &address);
  void And(Type Ty, GPRRegister dst, const Immediate &imm);
  void And(Type Ty, const AsmAddress &address, GPRRegister reg);
  void And(Type Ty, const AsmAddress &address, const Immediate &imm);

  void Or(Type Ty, GPRRegister dst, GPRRegister src);
  void Or(Type Ty, GPRRegister dst, const AsmAddress &address);
  void Or(Type Ty, GPRRegister dst, const Immediate &imm);
  void Or(Type Ty, const AsmAddress &address, GPRRegister reg);
  void Or(Type Ty, const AsmAddress &address, const Immediate &imm);

  void Xor(Type Ty, GPRRegister dst, GPRRegister src);
  void Xor(Type Ty, GPRRegister dst, const AsmAddress &address);
  void Xor(Type Ty, GPRRegister dst, const Immediate &imm);
  void Xor(Type Ty, const AsmAddress &address, GPRRegister reg);
  void Xor(Type Ty, const AsmAddress &address, const Immediate &imm);

  void add(Type Ty, GPRRegister dst, GPRRegister src);
  void add(Type Ty, GPRRegister reg, const AsmAddress &address);
  void add(Type Ty, GPRRegister reg, const Immediate &imm);
  void add(Type Ty, const AsmAddress &address, GPRRegister reg);
  void add(Type Ty, const AsmAddress &address, const Immediate &imm);

  void adc(Type Ty, GPRRegister dst, GPRRegister src);
  void adc(Type Ty, GPRRegister dst, const AsmAddress &address);
  void adc(Type Ty, GPRRegister reg, const Immediate &imm);
  void adc(Type Ty, const AsmAddress &address, GPRRegister reg);
  void adc(Type Ty, const AsmAddress &address, const Immediate &imm);

  void sub(Type Ty, GPRRegister dst, GPRRegister src);
  void sub(Type Ty, GPRRegister reg, const AsmAddress &address);
  void sub(Type Ty, GPRRegister reg, const Immediate &imm);
  void sub(Type Ty, const AsmAddress &address, GPRRegister reg);
  void sub(Type Ty, const AsmAddress &address, const Immediate &imm);

  void sbb(Type Ty, GPRRegister dst, GPRRegister src);
  void sbb(Type Ty, GPRRegister reg, const AsmAddress &address);
  void sbb(Type Ty, GPRRegister reg, const Immediate &imm);
  void sbb(Type Ty, const AsmAddress &address, GPRRegister reg);
  void sbb(Type Ty, const AsmAddress &address, const Immediate &imm);

  void cbw();
  void cwd();
  void cdq();
  void cqo();

  void div(Type Ty, GPRRegister reg);
  void div(Type Ty, const AsmAddress &address);

  void idiv(Type Ty, GPRRegister reg);
  void idiv(Type Ty, const AsmAddress &address);

  void imul(Type Ty, GPRRegister dst, GPRRegister src);
  void imul(Type Ty, GPRRegister reg, const Immediate &imm);
  void imul(Type Ty, GPRRegister reg, const AsmAddress &address);

  void imul(Type Ty, GPRRegister reg);
  void imul(Type Ty, const AsmAddress &address);

  void imul(Type Ty, GPRRegister dst, GPRRegister src, const Immediate &imm);
  void imul(Type Ty, GPRRegister dst, const AsmAddress &address,
            const Immediate &imm);

  void mul(Type Ty, GPRRegister reg);
  void mul(Type Ty, const AsmAddress &address);

  void incl(GPRRegister reg);
  void incl(const AsmAddress &address);

  void decl(GPRRegister reg);
  void decl(const AsmAddress &address);

  void rol(Type Ty, GPRRegister reg, const Immediate &imm);
  void rol(Type Ty, GPRRegister operand, GPRRegister shifter);
  void rol(Type Ty, const AsmAddress &operand, GPRRegister shifter);

  void shl(Type Ty, GPRRegister reg, const Immediate &imm);
  void shl(Type Ty, GPRRegister operand, GPRRegister shifter);
  void shl(Type Ty, const AsmAddress &operand, GPRRegister shifter);

  void shr(Type Ty, GPRRegister reg, const Immediate &imm);
  void shr(Type Ty, GPRRegister operand, GPRRegister shifter);
  void shr(Type Ty, const AsmAddress &operand, GPRRegister shifter);

  void sar(Type Ty, GPRRegister reg, const Immediate &imm);
  void sar(Type Ty, GPRRegister operand, GPRRegister shifter);
  void sar(Type Ty, const AsmAddress &address, GPRRegister shifter);

  void shld(Type Ty, GPRRegister dst, GPRRegister src);
  void shld(Type Ty, GPRRegister dst, GPRRegister src, const Immediate &imm);
  void shld(Type Ty, const AsmAddress &operand, GPRRegister src);
  void shrd(Type Ty, GPRRegister dst, GPRRegister src);
  void shrd(Type Ty, GPRRegister dst, GPRRegister src, const Immediate &imm);
  void shrd(Type Ty, const AsmAddress &dst, GPRRegister src);

  void neg(Type Ty, GPRRegister reg);
  void neg(Type Ty, const AsmAddress &addr);
  void notl(GPRRegister reg);

  void bsf(Type Ty, GPRRegister dst, GPRRegister src);
  void bsf(Type Ty, GPRRegister dst, const AsmAddress &src);
  void bsr(Type Ty, GPRRegister dst, GPRRegister src);
  void bsr(Type Ty, GPRRegister dst, const AsmAddress &src);

  void bswap(Type Ty, GPRRegister reg);

  void bt(GPRRegister base, GPRRegister offset);

  void ret();
  void ret(const Immediate &imm);

  // 'size' indicates size in bytes and must be in the range 1..8.
  void nop(int size = 1);
  void int3();
  void hlt();
  void ud2();

  // j(Label) is fully tested.
  void j(BrCond condition, Label *label, bool near = kFarJump);
  void j(BrCond condition, const ConstantRelocatable *label); // not testable.

  void jmp(GPRRegister reg);
  void jmp(Label *label, bool near = kFarJump);
  void jmp(const ConstantRelocatable *label); // not testable.
  void jmp(const Immediate &abs_address);

  void mfence();

  void lock();
  void cmpxchg(Type Ty, const AsmAddress &address, GPRRegister reg,
               bool Locked);
  void cmpxchg8b(const AsmAddress &address, bool Locked);
  void xadd(Type Ty, const AsmAddress &address, GPRRegister reg, bool Locked);
  void xchg(Type Ty, GPRRegister reg0, GPRRegister reg1);
  void xchg(Type Ty, const AsmAddress &address, GPRRegister reg);

  /// \name Intel Architecture Code Analyzer markers.
  /// @{
  void iaca_start();
  void iaca_end();
  /// @}

  void emitSegmentOverride(uint8_t prefix);

  intptr_t preferredLoopAlignment() { return 16; }
  void align(intptr_t alignment, intptr_t offset);
  void bind(Label *label);

  intptr_t CodeSize() const { return Buffer.size(); }

protected:
  inline void emitUint8(uint8_t value);

private:
  ENABLE_MAKE_UNIQUE;

  static constexpr Type RexTypeIrrelevant = IceType_i32;
  static constexpr Type RexTypeForceRexW = IceType_i64;
  static constexpr GPRRegister RexRegIrrelevant = GPRRegister::Encoded_Reg_eax;

  inline void emitInt16(int16_t value);
  inline void emitInt32(int32_t value);
  inline void emitRegisterOperand(int rm, int reg);
  template <typename RegType, typename RmType>
  inline void emitXmmRegisterOperand(RegType reg, RmType rm);
  inline void emitOperandSizeOverride();

  void emitOperand(int rm, const AsmOperand &operand, RelocOffsetT Addend = 0);
  void emitImmediate(Type ty, const Immediate &imm);
  void emitComplexI8(int rm, const AsmOperand &operand,
                     const Immediate &immediate);
  void emitComplex(Type Ty, int rm, const AsmOperand &operand,
                   const Immediate &immediate);
  void emitLabel(Label *label, intptr_t instruction_size);
  void emitLabelLink(Label *label);
  void emitNearLabelLink(Label *label);

  void emitGenericShift(int rm, Type Ty, GPRRegister reg, const Immediate &imm);
  void emitGenericShift(int rm, Type Ty, const AsmOperand &operand,
                        GPRRegister shifter);

  using LabelVector = std::vector<Label *>;
  // A vector of pool-allocated x86 labels for CFG nodes.
  LabelVector CfgNodeLabels;
  // A vector of pool-allocated x86 labels for Local labels.
  LabelVector LocalLabels;

  Label *getOrCreateLabel(SizeT Number, LabelVector &Labels);

  // The arith_int() methods factor out the commonality between the encodings
  // of add(), Or(), adc(), sbb(), And(), sub(), Xor(), and cmp(). The Tag
  // parameter is statically asserted to be less than 8.
  template <uint32_t Tag>
  void arith_int(Type Ty, GPRRegister reg, const Immediate &imm);

  template <uint32_t Tag>
  void arith_int(Type Ty, GPRRegister reg0, GPRRegister reg1);

  template <uint32_t Tag>
  void arith_int(Type Ty, GPRRegister reg, const AsmAddress &address);

  template <uint32_t Tag>
  void arith_int(Type Ty, const AsmAddress &address, GPRRegister reg);

  template <uint32_t Tag>
  void arith_int(Type Ty, const AsmAddress &address, const Immediate &imm);

  // gprEncoding returns Reg encoding for operand emission. For x86-64 we mask
  // out the 4th bit as it is encoded in the REX.[RXB] bits. No other bits are
  // touched because we don't want to mask errors.
  template <typename RegType> GPRRegister gprEncoding(const RegType Reg) {
    return static_cast<GPRRegister>(static_cast<uint8_t>(Reg) & ~0x08);
  }

  template <typename RegType>
  bool is8BitRegisterRequiringRex(const Type Ty, const RegType Reg) {
    static constexpr bool IsGPR =
        std::is_same<typename std::decay<RegType>::type, ByteRegister>::value ||
        std::is_same<typename std::decay<RegType>::type, GPRRegister>::value;

    // At this point in the assembler, we have encoded regs, so it is not
    // possible to distinguish between the "new" low byte registers introduced
    // in x86-64 and the legacy [abcd]h registers. Because x86, we may still
    // see ah (div) in the assembler, so we allow it here.
    //
    // The "local" uint32_t Encoded_Reg_ah is needed because RegType is an
    // enum that is not necessarily the same type of
    // RegisterSet::Encoded_Reg_ah.
    constexpr uint32_t Encoded_Reg_ah = RegisterSet::Encoded_Reg_ah;
    return IsGPR && (Reg & 0x04) != 0 && (Reg & 0x08) == 0 &&
           isByteSizedType(Ty) && (Reg != Encoded_Reg_ah);
  }

  // assembleAndEmitRex is used for determining which (if any) rex prefix
  // should be emitted for the current instruction. It allows different types
  // for Reg and Rm because they could be of different types (e.g., in
  // mov[sz]x instructions.) If Addr is not nullptr, then Rm is ignored, and
  // Rex.B is determined by Addr instead. TyRm is still used to determine
  // Addr's size.
  template <typename RegType, typename RmType>
  void assembleAndEmitRex(const Type TyReg, const RegType Reg, const Type TyRm,
                          const RmType Rm, const AsmAddress *Addr = nullptr) {
    const uint8_t W = (TyReg == IceType_i64 || TyRm == IceType_i64)
                          ? AsmOperand::RexW
                          : AsmOperand::RexNone;
    const uint8_t R = (Reg & 0x08) ? AsmOperand::RexR : AsmOperand::RexNone;
    const uint8_t X = (Addr != nullptr) ? (AsmOperand::RexBits)Addr->rexX()
                                        : AsmOperand::RexNone;
    const uint8_t B = (Addr != nullptr) ? (AsmOperand::RexBits)Addr->rexB()
                      : (Rm & 0x08)     ? AsmOperand::RexB
                                        : AsmOperand::RexNone;
    const uint8_t Prefix = W | R | X | B;
    if (Prefix != AsmOperand::RexNone) {
      emitUint8(Prefix);
    } else if (is8BitRegisterRequiringRex(TyReg, Reg) ||
               (Addr == nullptr && is8BitRegisterRequiringRex(TyRm, Rm))) {
      emitUint8(AsmOperand::RexBase);
    }
  }

  // emitRexRB is used for emitting a Rex prefix instructions with two
  // explicit register operands in its mod-rm byte.
  template <typename RegType, typename RmType>
  void emitRexRB(const Type Ty, const RegType Reg, const RmType Rm) {
    assembleAndEmitRex(Ty, Reg, Ty, Rm);
  }

  template <typename RegType, typename RmType>
  void emitRexRB(const Type TyReg, const RegType Reg, const Type TyRm,
                 const RmType Rm) {
    assembleAndEmitRex(TyReg, Reg, TyRm, Rm);
  }

  // emitRexB is used for emitting a Rex prefix if one is needed on encoding
  // the Reg field in an x86 instruction. It is invoked by the template when
  // Reg is the single register operand in the instruction (e.g., push Reg.)
  template <typename RmType> void emitRexB(const Type Ty, const RmType Rm) {
    emitRexRB(Ty, RexRegIrrelevant, Ty, Rm);
  }

  // emitRex is used for emitting a Rex prefix for an address and a GPR. The
  // address may contain zero, one, or two registers.
  template <typename RegType>
  void emitRex(const Type Ty, const AsmAddress &Addr, const RegType Reg) {
    assembleAndEmitRex(Ty, Reg, Ty, RexRegIrrelevant, &Addr);
  }

  template <typename RegType>
  void emitRex(const Type AddrTy, const AsmAddress &Addr, const Type TyReg,
               const RegType Reg) {
    assembleAndEmitRex(TyReg, Reg, AddrTy, RexRegIrrelevant, &Addr);
  }
};

inline void AssemblerX8664::emitUint8(uint8_t value) {
  Buffer.emit<uint8_t>(value);
}

inline void AssemblerX8664::emitInt16(int16_t value) {
  Buffer.emit<int16_t>(value);
}

inline void AssemblerX8664::emitInt32(int32_t value) {
  Buffer.emit<int32_t>(value);
}

inline void AssemblerX8664::emitRegisterOperand(int reg, int rm) {
  assert(reg >= 0 && reg < 8);
  assert(rm >= 0 && rm < 8);
  Buffer.emit<uint8_t>(0xC0 + (reg << 3) + rm);
}

template <typename RegType, typename RmType>
inline void AssemblerX8664::emitXmmRegisterOperand(RegType reg, RmType rm) {
  emitRegisterOperand(gprEncoding(reg), gprEncoding(rm));
}

inline void AssemblerX8664::emitOperandSizeOverride() { emitUint8(0x66); }

using Label = AssemblerX8664::Label;
using Immediate = AssemblerX8664::Immediate;

} // end of namespace X8664
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEASSEMBLERX8664_H
