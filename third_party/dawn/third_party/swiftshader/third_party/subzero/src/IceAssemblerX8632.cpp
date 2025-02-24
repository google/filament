//===- subzero/src/IceAssemblerX8632Impl.h - base x86 assembler -*- C++ -*-=//
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
//
/// \file
/// \brief Implements the AssemblerX8632 class.
//
//===----------------------------------------------------------------------===//

#include "IceAssemblerX8632.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceOperand.h"
#include "IceTargetLoweringX8632.h"

namespace Ice {
namespace X8632 {

AsmAddress::AsmAddress(const Variable *Var, const TargetX8632 *Target) {
  if (Var->hasReg())
    llvm::report_fatal_error("Stack Variable has a register assigned");
  if (Var->mustHaveReg()) {
    llvm::report_fatal_error("Infinite-weight Variable (" + Var->getName() +
                             ") has no register assigned - function " +
                             Target->getFunc()->getFunctionName());
  }
  int32_t Offset = Var->getStackOffset();
  auto BaseRegNum = Var->getBaseRegNum();
  if (Var->getBaseRegNum().hasNoValue()) {
    // If the stack pointer needs alignment, we must use the frame pointer
    // for arguments. For locals, getFrameOrStackReg will return the stack
    // pointer in this case.
    if (Target->needsStackPointerAlignment() && Var->getIsArg()) {
      assert(Target->hasFramePointer());
      BaseRegNum = Target->getFrameReg();
    } else {
      BaseRegNum = Target->getFrameOrStackReg();
    }
  }

  GPRRegister Base = RegX8632::getEncodedGPR(BaseRegNum);

  if (Utils::IsInt(8, Offset)) {
    SetModRM(1, Base);
    if (Base == RegX8632::Encoded_Reg_esp)
      SetSIB(TIMES_1, RegX8632::Encoded_Reg_esp, Base);
    SetDisp8(Offset);
  } else {
    SetModRM(2, Base);
    if (Base == RegX8632::Encoded_Reg_esp)
      SetSIB(TIMES_1, RegX8632::Encoded_Reg_esp, Base);
    SetDisp32(Offset);
  }
}

AsmAddress::AsmAddress(const X86OperandMem *Mem, Ice::Assembler *Asm,
                       const Ice::TargetLowering *Target) {
  Mem->validateMemOperandPIC();
  int32_t Disp = 0;
  if (Mem->getBase() && Mem->getBase()->isRematerializable()) {
    Disp += Mem->getBase()->getRematerializableOffset(Target);
  }
  // The index should never be rematerializable.  But if we ever allow it, then
  // we should make sure the rematerialization offset is shifted by the Shift
  // value.
  assert(!Mem->getIndex() || !Mem->getIndex()->isRematerializable());

  AssemblerFixup *Fixup = nullptr;
  // Determine the offset (is it relocatable?)
  if (Mem->getOffset()) {
    if (const auto *CI = llvm::dyn_cast<ConstantInteger32>(Mem->getOffset())) {
      Disp += static_cast<int32_t>(CI->getValue());
    } else if (const auto CR =
                   llvm::dyn_cast<ConstantRelocatable>(Mem->getOffset())) {
      Disp += CR->getOffset();
      Fixup = Asm->createFixup(FK_Abs, CR);
    } else {
      llvm_unreachable("Unexpected offset type");
    }
  }

  // Now convert to the various possible forms.
  if (Mem->getBase() && Mem->getIndex()) {
    SetBaseIndex(RegX8632::getEncodedGPR(Mem->getBase()->getRegNum()),
                 RegX8632::getEncodedGPR(Mem->getIndex()->getRegNum()),
                 ScaleFactor(Mem->getShift()), Disp, Fixup);
  } else if (Mem->getBase()) {
    SetBase(RegX8632::getEncodedGPR(Mem->getBase()->getRegNum()), Disp, Fixup);
  } else if (Mem->getIndex()) {
    SetIndex(RegX8632::getEncodedGPR(Mem->getIndex()->getRegNum()),
             ScaleFactor(Mem->getShift()), Disp, Fixup);
  } else {
    SetAbsolute(Disp, Fixup);
  }
}

AsmAddress::AsmAddress(const VariableSplit *Split, const Cfg *Func) {
  assert(!Split->getVar()->hasReg());
  const ::Ice::TargetLowering *Target = Func->getTarget();
  int32_t Offset = Split->getVar()->getStackOffset() + Split->getOffset();
  SetBase(RegX8632::getEncodedGPR(Target->getFrameOrStackReg()), Offset,
          AssemblerFixup::NoFixup);
}

AssemblerX8632::~AssemblerX8632() {
  if (BuildDefs::asserts()) {
    for (const Label *Label : CfgNodeLabels) {
      Label->finalCheck();
    }
    for (const Label *Label : LocalLabels) {
      Label->finalCheck();
    }
  }
}

void AssemblerX8632::alignFunction() {
  const SizeT Align = 1 << getBundleAlignLog2Bytes();
  SizeT BytesNeeded = Utils::OffsetToAlignment(Buffer.getPosition(), Align);
  constexpr SizeT HltSize = 1;
  while (BytesNeeded > 0) {
    hlt();
    BytesNeeded -= HltSize;
  }
}

AssemblerX8632::Label *AssemblerX8632::getOrCreateLabel(SizeT Number,
                                                        LabelVector &Labels) {
  Label *L = nullptr;
  if (Number == Labels.size()) {
    L = new (this->allocate<Label>()) Label();
    Labels.push_back(L);
    return L;
  }
  if (Number > Labels.size()) {
    Utils::reserveAndResize(Labels, Number + 1);
  }
  L = Labels[Number];
  if (!L) {
    L = new (this->allocate<Label>()) Label();
    Labels[Number] = L;
  }
  return L;
}

Ice::Label *AssemblerX8632::getCfgNodeLabel(SizeT NodeNumber) {
  assert(NodeNumber < CfgNodeLabels.size());
  return CfgNodeLabels[NodeNumber];
}

AssemblerX8632::Label *
AssemblerX8632::getOrCreateCfgNodeLabel(SizeT NodeNumber) {
  return getOrCreateLabel(NodeNumber, CfgNodeLabels);
}

AssemblerX8632::Label *AssemblerX8632::getOrCreateLocalLabel(SizeT Number) {
  return getOrCreateLabel(Number, LocalLabels);
}

void AssemblerX8632::bindCfgNodeLabel(const CfgNode *Node) {
  assert(!getPreliminary());
  Label *L = getOrCreateCfgNodeLabel(Node->getIndex());
  this->bind(L);
}

void AssemblerX8632::bindLocalLabel(SizeT Number) {
  Label *L = getOrCreateLocalLabel(Number);
  if (!getPreliminary())
    this->bind(L);
}

void AssemblerX8632::call(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xFF);
  emitRegisterOperand(2, gprEncoding(reg));
}

void AssemblerX8632::call(const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xFF);
  emitOperand(2, address);
}

void AssemblerX8632::call(const ConstantRelocatable *label) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  intptr_t call_start = Buffer.getPosition();
  emitUint8(0xE8);
  auto *Fixup = this->createFixup(FK_PcRel, label);
  Fixup->set_addend(-4);
  emitFixup(Fixup);
  emitInt32(0);
  assert((Buffer.getPosition() - call_start) == kCallExternalLabelSize);
  (void)call_start;
}

void AssemblerX8632::call(const Immediate &abs_address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  intptr_t call_start = Buffer.getPosition();
  emitUint8(0xE8);
  auto *Fixup = this->createFixup(FK_PcRel, AssemblerFixup::NullSymbol);
  Fixup->set_addend(abs_address.value() - 4);
  emitFixup(Fixup);
  emitInt32(0);
  assert((Buffer.getPosition() - call_start) == kCallExternalLabelSize);
  (void)call_start;
}

void AssemblerX8632::pushl(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x50 + gprEncoding(reg));
}

void AssemblerX8632::pushl(const Immediate &Imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x68);
  emitInt32(Imm.value());
}

void AssemblerX8632::pushl(const ConstantRelocatable *Label) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x68);
  emitFixup(this->createFixup(FK_Abs, Label));
  // In x86-32, the emitted value is an addend to the relocation. Therefore, we
  // must emit a 0 (because we're pushing an absolute relocation.)
  // In x86-64, the emitted value does not matter (the addend lives in the
  // relocation record as an extra field.)
  emitInt32(0);
}

void AssemblerX8632::popl(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // Any type that would not force a REX prefix to be emitted can be provided
  // here.
  emitUint8(0x58 + gprEncoding(reg));
}

void AssemblerX8632::popl(const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x8F);
  emitOperand(0, address);
}

void AssemblerX8632::pushal() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x60);
}

void AssemblerX8632::popal() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x61);
}

void AssemblerX8632::setcc(BrCond condition, ByteRegister dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x90 + condition);
  emitUint8(0xC0 + gprEncoding(dst));
}

void AssemblerX8632::setcc(BrCond condition, const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x90 + condition);
  emitOperand(0, address);
}

void AssemblerX8632::mov(Type Ty, GPRRegister dst, const Immediate &imm) {
  assert(Ty != IceType_i64 && "i64 not supported yet.");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitUint8(0xB0 + gprEncoding(dst));
    emitUint8(imm.value() & 0xFF);
  } else {
    // TODO(jpp): When removing the assertion above ensure that in x86-64 we
    // emit a 64-bit immediate.
    emitUint8(0xB8 + gprEncoding(dst));
    emitImmediate(Ty, imm);
  }
}

void AssemblerX8632::mov(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitUint8(0x88);
  } else {
    emitUint8(0x89);
  }
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
}

void AssemblerX8632::mov(Type Ty, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitUint8(0x8A);
  } else {
    emitUint8(0x8B);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::mov(Type Ty, const AsmAddress &dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitUint8(0x88);
  } else {
    emitUint8(0x89);
  }
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::mov(Type Ty, const AsmAddress &dst, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitUint8(0xC6);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(0, dst, OffsetFromNextInstruction);
    emitUint8(imm.value() & 0xFF);
  } else {
    emitUint8(0xC7);
    const uint8_t OffsetFromNextInstruction = Ty == IceType_i16 ? 2 : 4;
    emitOperand(0, dst, OffsetFromNextInstruction);
    emitImmediate(Ty, imm);
  }
}

void AssemblerX8632::movzx(Type SrcTy, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  bool ByteSized = isByteSizedType(SrcTy);
  assert(ByteSized || SrcTy == IceType_i16);
  emitUint8(0x0F);
  emitUint8(ByteSized ? 0xB6 : 0xB7);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::movzx(Type SrcTy, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  bool ByteSized = isByteSizedType(SrcTy);
  assert(ByteSized || SrcTy == IceType_i16);
  emitUint8(0x0F);
  emitUint8(ByteSized ? 0xB6 : 0xB7);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::movsx(Type SrcTy, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  bool ByteSized = isByteSizedType(SrcTy);
  assert(ByteSized || SrcTy == IceType_i16);
  emitUint8(0x0F);
  emitUint8(ByteSized ? 0xBE : 0xBF);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::movsx(Type SrcTy, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  bool ByteSized = isByteSizedType(SrcTy);
  assert(ByteSized || SrcTy == IceType_i16);
  emitUint8(0x0F);
  emitUint8(ByteSized ? 0xBE : 0xBF);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::lea(Type Ty, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x8D);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cmov(Type Ty, BrCond cond, GPRRegister dst,
                          GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  else
    assert(Ty == IceType_i32);
  emitUint8(0x0F);
  emitUint8(0x40 + cond);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::cmov(Type Ty, BrCond cond, GPRRegister dst,
                          const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  else
    assert(Ty == IceType_i32);
  emitUint8(0x0F);
  emitUint8(0x40 + cond);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::rep_movsb() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF3);
  emitUint8(0xA4);
}

void AssemblerX8632::movss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x10);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::movss(Type Ty, const AsmAddress &dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x11);
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::movss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x11);
  emitXmmRegisterOperand(src, dst);
}

void AssemblerX8632::movd(Type SrcTy, XmmRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x6E);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::movd(Type SrcTy, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x6E);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::movd(Type DestTy, GPRRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x7E);
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
}

void AssemblerX8632::movd(Type DestTy, const AsmAddress &dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x7E);
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::movq(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF3);
  emitUint8(0x0F);
  emitUint8(0x7E);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::movq(const AsmAddress &dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xD6);
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::movq(XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF3);
  emitUint8(0x0F);
  emitUint8(0x7E);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::addss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x58);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::addss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x58);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::subss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5C);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::subss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5C);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::mulss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x59);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::mulss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x59);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::divss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5E);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::divss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5E);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::fld(Type Ty, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xD9 : 0xDD);
  emitOperand(0, src);
}

void AssemblerX8632::fstp(Type Ty, const AsmAddress &dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xD9 : 0xDD);
  emitOperand(3, dst);
}

void AssemblerX8632::fstp(RegX8632::X87STRegister st) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xDD);
  emitUint8(0xD8 + st);
}

void AssemblerX8632::movaps(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x28);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::movups(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x10);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::movups(XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x10);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::movups(const AsmAddress &dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x11);
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::padd(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xFC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xFD);
  } else {
    emitUint8(0xFE);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::padd(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xFC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xFD);
  } else {
    emitUint8(0xFE);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::padds(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xEC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xED);
  } else {
    assert(false && "Unexpected padds operand type");
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::padds(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xEC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xED);
  } else {
    assert(false && "Unexpected padds operand type");
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::paddus(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xDC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xDD);
  } else {
    assert(false && "Unexpected paddus operand type");
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::paddus(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xDC);
  } else if (Ty == IceType_i16) {
    emitUint8(0xDD);
  } else {
    assert(false && "Unexpected paddus operand type");
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pand(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xDB);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pand(Type /* Ty */, XmmRegister dst,
                          const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xDB);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pandn(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xDF);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pandn(Type /* Ty */, XmmRegister dst,
                           const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xDF);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pmull(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xD5);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0x38);
    emitUint8(0x40);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pmull(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xD5);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0x38);
    emitUint8(0x40);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pmulhw(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xE5);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pmulhw(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xE5);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pmulhuw(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xE4);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pmulhuw(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xE4);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pmaddwd(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xF5);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pmaddwd(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  assert(Ty == IceType_v8i16);
  (void)Ty;
  emitUint8(0xF5);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pmuludq(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xF4);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pmuludq(Type /* Ty */, XmmRegister dst,
                             const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xF4);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::por(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xEB);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::por(Type /* Ty */, XmmRegister dst,
                         const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xEB);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psub(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xF8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xF9);
  } else {
    emitUint8(0xFA);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psub(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xF8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xF9);
  } else {
    emitUint8(0xFA);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psubs(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xE8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xE9);
  } else {
    assert(false && "Unexpected psubs operand type");
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psubs(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xE8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xE9);
  } else {
    assert(false && "Unexpected psubs operand type");
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psubus(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xD8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xD9);
  } else {
    assert(false && "Unexpected psubus operand type");
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psubus(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0xD8);
  } else if (Ty == IceType_i16) {
    emitUint8(0xD9);
  } else {
    assert(false && "Unexpected psubus operand type");
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pxor(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xEF);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pxor(Type /* Ty */, XmmRegister dst,
                          const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xEF);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psll(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xF1);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0xF2);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psll(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xF1);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0xF2);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psll(Type Ty, XmmRegister dst, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_int8());
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0x71);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0x72);
  }
  emitRegisterOperand(6, gprEncoding(dst));
  emitUint8(imm.value() & 0xFF);
}

void AssemblerX8632::psra(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xE1);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0xE2);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psra(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xE1);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0xE2);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psra(Type Ty, XmmRegister dst, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_int8());
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0x71);
  } else {
    assert(Ty == IceType_i32);
    emitUint8(0x72);
  }
  emitRegisterOperand(4, gprEncoding(dst));
  emitUint8(imm.value() & 0xFF);
}

void AssemblerX8632::psrl(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xD1);
  } else if (Ty == IceType_f64) {
    emitUint8(0xD3);
  } else {
    assert(Ty == IceType_i32 || Ty == IceType_f32 || Ty == IceType_v4f32);
    emitUint8(0xD2);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::psrl(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xD1);
  } else if (Ty == IceType_f64) {
    emitUint8(0xD3);
  } else {
    assert(Ty == IceType_i32 || Ty == IceType_f32 || Ty == IceType_v4f32);
    emitUint8(0xD2);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::psrl(Type Ty, XmmRegister dst, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_int8());
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0x71);
  } else if (Ty == IceType_f64) {
    emitUint8(0x73);
  } else {
    assert(Ty == IceType_i32 || Ty == IceType_f32 || Ty == IceType_v4f32);
    emitUint8(0x72);
  }
  emitRegisterOperand(2, gprEncoding(dst));
  emitUint8(imm.value() & 0xFF);
}

// {add,sub,mul,div}ps are given a Ty parameter for consistency with
// {add,sub,mul,div}ss. In the future, when the PNaCl ABI allows addpd, etc.,
// we can use the Ty parameter to decide on adding a 0x66 prefix.

void AssemblerX8632::addps(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x58);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::addps(Type /* Ty */, XmmRegister dst,
                           const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x58);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::subps(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5C);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::subps(Type /* Ty */, XmmRegister dst,
                           const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5C);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::divps(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5E);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::divps(Type /* Ty */, XmmRegister dst,
                           const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5E);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::mulps(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x59);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::mulps(Type /* Ty */, XmmRegister dst,
                           const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x59);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::minps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5D);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::minps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5D);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::minss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5D);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::minss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5D);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::maxps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5F);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::maxps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5F);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::maxss(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5F);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::maxss(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5F);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::andnps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x55);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::andnps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x55);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::andps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x54);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::andps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x54);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::orps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x56);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::orps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x56);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::blendvps(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x14);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::blendvps(Type /* Ty */, XmmRegister dst,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x14);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pblendvb(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x10);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pblendvb(Type /* Ty */, XmmRegister dst,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x10);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cmpps(Type Ty, XmmRegister dst, XmmRegister src,
                           CmppsCond CmpCondition) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_f64)
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xC2);
  emitXmmRegisterOperand(dst, src);
  emitUint8(CmpCondition);
}

void AssemblerX8632::cmpps(Type Ty, XmmRegister dst, const AsmAddress &src,
                           CmppsCond CmpCondition) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_f64)
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0xC2);
  static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
  emitOperand(gprEncoding(dst), src, OffsetFromNextInstruction);
  emitUint8(CmpCondition);
}

void AssemblerX8632::sqrtps(XmmRegister dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x51);
  emitXmmRegisterOperand(dst, dst);
}

void AssemblerX8632::rsqrtps(XmmRegister dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x52);
  emitXmmRegisterOperand(dst, dst);
}

void AssemblerX8632::reciprocalps(XmmRegister dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x53);
  emitXmmRegisterOperand(dst, dst);
}

void AssemblerX8632::movhlps(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x12);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::movlhps(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x16);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::unpcklps(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x14);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::unpckhps(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x15);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::unpcklpd(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x14);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::unpckhpd(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x15);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::set1ps(XmmRegister dst, GPRRegister tmp1,
                            const Immediate &imm) {
  // Load 32-bit immediate value into tmp1.
  mov(IceType_i32, tmp1, imm);
  // Move value from tmp1 into dst.
  movd(IceType_i32, dst, tmp1);
  // Broadcast low lane into other three lanes.
  shufps(RexTypeIrrelevant, dst, dst, Immediate(0x0));
}

void AssemblerX8632::pshufb(Type /* Ty */, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x00);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pshufb(Type /* Ty */, XmmRegister dst,
                            const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x00);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pshufd(Type /* Ty */, XmmRegister dst, XmmRegister src,
                            const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x70);
  emitXmmRegisterOperand(dst, src);
  assert(imm.is_uint8());
  emitUint8(imm.value());
}

void AssemblerX8632::pshufd(Type /* Ty */, XmmRegister dst,
                            const AsmAddress &src, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x70);
  static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
  emitOperand(gprEncoding(dst), src, OffsetFromNextInstruction);
  assert(imm.is_uint8());
  emitUint8(imm.value());
}

void AssemblerX8632::punpckl(Type Ty, XmmRegister Dst, XmmRegister Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x62);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x61);
  } else if (Ty == IceType_v16i8) {
    emitUint8(0x60);
  } else {
    assert(false && "Unexpected vector unpack operand type");
  }
  emitXmmRegisterOperand(Dst, Src);
}

void AssemblerX8632::punpckl(Type Ty, XmmRegister Dst, const AsmAddress &Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x62);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x61);
  } else if (Ty == IceType_v16i8) {
    emitUint8(0x60);
  } else {
    assert(false && "Unexpected vector unpack operand type");
  }
  emitOperand(gprEncoding(Dst), Src);
}

void AssemblerX8632::punpckh(Type Ty, XmmRegister Dst, XmmRegister Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x6A);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x69);
  } else if (Ty == IceType_v16i8) {
    emitUint8(0x68);
  } else {
    assert(false && "Unexpected vector unpack operand type");
  }
  emitXmmRegisterOperand(Dst, Src);
}

void AssemblerX8632::punpckh(Type Ty, XmmRegister Dst, const AsmAddress &Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x6A);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x69);
  } else if (Ty == IceType_v16i8) {
    emitUint8(0x68);
  } else {
    assert(false && "Unexpected vector unpack operand type");
  }
  emitOperand(gprEncoding(Dst), Src);
}

void AssemblerX8632::packss(Type Ty, XmmRegister Dst, XmmRegister Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x6B);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x63);
  } else {
    assert(false && "Unexpected vector pack operand type");
  }
  emitXmmRegisterOperand(Dst, Src);
}

void AssemblerX8632::packss(Type Ty, XmmRegister Dst, const AsmAddress &Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x6B);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x63);
  } else {
    assert(false && "Unexpected vector pack operand type");
  }
  emitOperand(gprEncoding(Dst), Src);
}

void AssemblerX8632::packus(Type Ty, XmmRegister Dst, XmmRegister Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x38);
    emitUint8(0x2B);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x67);
  } else {
    assert(false && "Unexpected vector pack operand type");
  }
  emitXmmRegisterOperand(Dst, Src);
}

void AssemblerX8632::packus(Type Ty, XmmRegister Dst, const AsmAddress &Src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_v4i32 || Ty == IceType_v4f32) {
    emitUint8(0x38);
    emitUint8(0x2B);
  } else if (Ty == IceType_v8i16) {
    emitUint8(0x67);
  } else {
    assert(false && "Unexpected vector pack operand type");
  }
  emitOperand(gprEncoding(Dst), Src);
}

void AssemblerX8632::shufps(Type /* Ty */, XmmRegister dst, XmmRegister src,
                            const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0xC6);
  emitXmmRegisterOperand(dst, src);
  assert(imm.is_uint8());
  emitUint8(imm.value());
}

void AssemblerX8632::shufps(Type /* Ty */, XmmRegister dst,
                            const AsmAddress &src, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0xC6);
  static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
  emitOperand(gprEncoding(dst), src, OffsetFromNextInstruction);
  assert(imm.is_uint8());
  emitUint8(imm.value());
}

void AssemblerX8632::sqrtpd(XmmRegister dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x51);
  emitXmmRegisterOperand(dst, dst);
}

void AssemblerX8632::cvtdq2ps(Type /* Ignore */, XmmRegister dst,
                              XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvtdq2ps(Type /* Ignore */, XmmRegister dst,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvttps2dq(Type /* Ignore */, XmmRegister dst,
                               XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF3);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvttps2dq(Type /* Ignore */, XmmRegister dst,
                               const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF3);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvtps2dq(Type /* Ignore */, XmmRegister dst,
                              XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvtps2dq(Type /* Ignore */, XmmRegister dst,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x5B);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvtsi2ss(Type DestTy, XmmRegister dst, Type SrcTy,
                              GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(DestTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2A);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvtsi2ss(Type DestTy, XmmRegister dst, Type SrcTy,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(DestTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2A);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvtfloat2float(Type SrcTy, XmmRegister dst,
                                    XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // ss2sd or sd2ss
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5A);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvtfloat2float(Type SrcTy, XmmRegister dst,
                                    const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x5A);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvttss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                               XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2C);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvttss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                               const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2C);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::cvtss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                              XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2D);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::cvtss2si(Type DestTy, GPRRegister dst, Type SrcTy,
                              const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(isFloat32Asserting32Or64(SrcTy) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x2D);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::ucomiss(Type Ty, XmmRegister a, XmmRegister b) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_f64)
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x2E);
  emitXmmRegisterOperand(a, b);
}

void AssemblerX8632::ucomiss(Type Ty, XmmRegister a, const AsmAddress &b) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_f64)
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x2E);
  emitOperand(gprEncoding(a), b);
}

void AssemblerX8632::movmsk(Type Ty, GPRRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_v16i8) {
    emitUint8(0x66);
  } else if (Ty == IceType_v4f32 || Ty == IceType_v4i32) {
    // No operand size prefix
  } else {
    assert(false && "Unexpected movmsk operand type");
  }
  emitUint8(0x0F);
  if (Ty == IceType_v16i8) {
    emitUint8(0xD7);
  } else if (Ty == IceType_v4f32 || Ty == IceType_v4i32) {
    emitUint8(0x50);
  } else {
    assert(false && "Unexpected movmsk operand type");
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::sqrt(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (isScalarFloatingType(Ty))
    emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x51);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::sqrt(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (isScalarFloatingType(Ty))
    emitUint8(isFloat32Asserting32Or64(Ty) ? 0xF3 : 0xF2);
  emitUint8(0x0F);
  emitUint8(0x51);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::xorps(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x57);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::xorps(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (!isFloat32Asserting32Or64(Ty))
    emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x57);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::insertps(Type Ty, XmmRegister dst, XmmRegister src,
                              const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_uint8());
  assert(isVectorFloatingType(Ty));
  (void)Ty;
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x3A);
  emitUint8(0x21);
  emitXmmRegisterOperand(dst, src);
  emitUint8(imm.value());
}

void AssemblerX8632::insertps(Type Ty, XmmRegister dst, const AsmAddress &src,
                              const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_uint8());
  assert(isVectorFloatingType(Ty));
  (void)Ty;
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x3A);
  emitUint8(0x21);
  static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
  emitOperand(gprEncoding(dst), src, OffsetFromNextInstruction);
  emitUint8(imm.value());
}

void AssemblerX8632::pinsr(Type Ty, XmmRegister dst, GPRRegister src,
                           const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_uint8());
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xC4);
  } else {
    emitUint8(0x3A);
    emitUint8(isByteSizedType(Ty) ? 0x20 : 0x22);
  }
  emitXmmRegisterOperand(dst, src);
  emitUint8(imm.value());
}

void AssemblerX8632::pinsr(Type Ty, XmmRegister dst, const AsmAddress &src,
                           const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_uint8());
  emitUint8(0x66);
  emitUint8(0x0F);
  if (Ty == IceType_i16) {
    emitUint8(0xC4);
  } else {
    emitUint8(0x3A);
    emitUint8(isByteSizedType(Ty) ? 0x20 : 0x22);
  }
  static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
  emitOperand(gprEncoding(dst), src, OffsetFromNextInstruction);
  emitUint8(imm.value());
}

void AssemblerX8632::pextr(Type Ty, GPRRegister dst, XmmRegister src,
                           const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(imm.is_uint8());
  if (Ty == IceType_i16) {
    emitUint8(0x66);
    emitUint8(0x0F);
    emitUint8(0xC5);
    emitXmmRegisterOperand(dst, src);
    emitUint8(imm.value());
  } else {
    emitUint8(0x66);
    emitUint8(0x0F);
    emitUint8(0x3A);
    emitUint8(isByteSizedType(Ty) ? 0x14 : 0x16);
    // SSE 4.1 versions are "MRI" because dst can be mem, while pextrw (SSE2)
    // is RMI because dst must be reg.
    emitXmmRegisterOperand(src, dst);
    emitUint8(imm.value());
  }
}

void AssemblerX8632::pmovsxdq(XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x38);
  emitUint8(0x25);
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pcmpeq(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0x74);
  } else if (Ty == IceType_i16) {
    emitUint8(0x75);
  } else {
    emitUint8(0x76);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pcmpeq(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0x74);
  } else if (Ty == IceType_i16) {
    emitUint8(0x75);
  } else {
    emitUint8(0x76);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::pcmpgt(Type Ty, XmmRegister dst, XmmRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0x64);
  } else if (Ty == IceType_i16) {
    emitUint8(0x65);
  } else {
    emitUint8(0x66);
  }
  emitXmmRegisterOperand(dst, src);
}

void AssemblerX8632::pcmpgt(Type Ty, XmmRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty)) {
    emitUint8(0x64);
  } else if (Ty == IceType_i16) {
    emitUint8(0x65);
  } else {
    emitUint8(0x66);
  }
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::round(Type Ty, XmmRegister dst, XmmRegister src,
                           const Immediate &mode) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x3A);
  switch (Ty) {
  case IceType_v4f32:
    emitUint8(0x08);
    break;
  case IceType_f32:
    emitUint8(0x0A);
    break;
  case IceType_f64:
    emitUint8(0x0B);
    break;
  default:
    assert(false && "Unsupported round operand type");
  }
  emitXmmRegisterOperand(dst, src);
  // Mask precision exeption.
  emitUint8(static_cast<uint8_t>(mode.value()) | 0x8);
}

void AssemblerX8632::round(Type Ty, XmmRegister dst, const AsmAddress &src,
                           const Immediate &mode) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x66);
  emitUint8(0x0F);
  emitUint8(0x3A);
  switch (Ty) {
  case IceType_v4f32:
    emitUint8(0x08);
    break;
  case IceType_f32:
    emitUint8(0x0A);
    break;
  case IceType_f64:
    emitUint8(0x0B);
    break;
  default:
    assert(false && "Unsupported round operand type");
  }
  emitOperand(gprEncoding(dst), src);
  // Mask precision exeption.
  emitUint8(static_cast<uint8_t>(mode.value()) | 0x8);
}

void AssemblerX8632::fnstcw(const AsmAddress &dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xD9);
  emitOperand(7, dst);
}

void AssemblerX8632::fldcw(const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xD9);
  emitOperand(5, src);
}

void AssemblerX8632::fistpl(const AsmAddress &dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xDF);
  emitOperand(7, dst);
}

void AssemblerX8632::fistps(const AsmAddress &dst) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xDB);
  emitOperand(3, dst);
}

void AssemblerX8632::fildl(const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xDF);
  emitOperand(5, src);
}

void AssemblerX8632::filds(const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xDB);
  emitOperand(0, src);
}

void AssemblerX8632::fincstp() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xD9);
  emitUint8(0xF7);
}

template <uint32_t Tag>
void AssemblerX8632::arith_int(Type Ty, GPRRegister reg, const Immediate &imm) {
  static_assert(Tag < 8, "Tag must be between 0..7");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitComplexI8(Tag, AsmOperand(reg), imm);
  } else {
    emitComplex(Ty, Tag, AsmOperand(reg), imm);
  }
}

template <uint32_t Tag>
void AssemblerX8632::arith_int(Type Ty, GPRRegister reg0, GPRRegister reg1) {
  static_assert(Tag < 8, "Tag must be between 0..7");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty))
    emitUint8(Tag * 8 + 2);
  else
    emitUint8(Tag * 8 + 3);
  emitRegisterOperand(gprEncoding(reg0), gprEncoding(reg1));
}

template <uint32_t Tag>
void AssemblerX8632::arith_int(Type Ty, GPRRegister reg,
                               const AsmAddress &address) {
  static_assert(Tag < 8, "Tag must be between 0..7");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty))
    emitUint8(Tag * 8 + 2);
  else
    emitUint8(Tag * 8 + 3);
  emitOperand(gprEncoding(reg), address);
}

template <uint32_t Tag>
void AssemblerX8632::arith_int(Type Ty, const AsmAddress &address,
                               GPRRegister reg) {
  static_assert(Tag < 8, "Tag must be between 0..7");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty))
    emitUint8(Tag * 8 + 0);
  else
    emitUint8(Tag * 8 + 1);
  emitOperand(gprEncoding(reg), address);
}

template <uint32_t Tag>
void AssemblerX8632::arith_int(Type Ty, const AsmAddress &address,
                               const Immediate &imm) {
  static_assert(Tag < 8, "Tag must be between 0..7");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty)) {
    emitComplexI8(Tag, address, imm);
  } else {
    emitComplex(Ty, Tag, address, imm);
  }
}

void AssemblerX8632::cmp(Type Ty, GPRRegister reg, const Immediate &imm) {
  arith_int<7>(Ty, reg, imm);
}

void AssemblerX8632::cmp(Type Ty, GPRRegister reg0, GPRRegister reg1) {
  arith_int<7>(Ty, reg0, reg1);
}

void AssemblerX8632::cmp(Type Ty, GPRRegister reg, const AsmAddress &address) {
  arith_int<7>(Ty, reg, address);
}

void AssemblerX8632::cmp(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<7>(Ty, address, reg);
}

void AssemblerX8632::cmp(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<7>(Ty, address, imm);
}

void AssemblerX8632::test(Type Ty, GPRRegister reg1, GPRRegister reg2) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty))
    emitUint8(0x84);
  else
    emitUint8(0x85);
  emitRegisterOperand(gprEncoding(reg1), gprEncoding(reg2));
}

void AssemblerX8632::test(Type Ty, const AsmAddress &addr, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedType(Ty))
    emitUint8(0x84);
  else
    emitUint8(0x85);
  emitOperand(gprEncoding(reg), addr);
}

void AssemblerX8632::test(Type Ty, GPRRegister reg,
                          const Immediate &immediate) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // For registers that have a byte variant (EAX, EBX, ECX, and EDX) we only
  // test the byte register to keep the encoding short. This is legal even if
  // the register had high bits set since this only sets flags registers based
  // on the "AND" of the two operands, and the immediate had zeros at those
  // high bits.
  constexpr GPRRegister Last8BitGPR = GPRRegister::Encoded_Reg_ebx;
  if (immediate.is_uint8() && reg <= Last8BitGPR) {
    // Use zero-extended 8-bit immediate.
    if (reg == RegX8632::Encoded_Reg_eax) {
      emitUint8(0xA8);
    } else {
      emitUint8(0xF6);
      emitUint8(0xC0 + gprEncoding(reg));
    }
    emitUint8(immediate.value() & 0xFF);
  } else if (reg == RegX8632::Encoded_Reg_eax) {
    // Use short form if the destination is EAX.
    if (Ty == IceType_i16)
      emitOperandSizeOverride();
    emitUint8(0xA9);
    emitImmediate(Ty, immediate);
  } else {
    if (Ty == IceType_i16)
      emitOperandSizeOverride();
    emitUint8(0xF7);
    emitRegisterOperand(0, gprEncoding(reg));
    emitImmediate(Ty, immediate);
  }
}

void AssemblerX8632::test(Type Ty, const AsmAddress &addr,
                          const Immediate &immediate) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // If the immediate is short, we only test the byte addr to keep the encoding
  // short.
  if (immediate.is_uint8()) {
    // Use zero-extended 8-bit immediate.
    emitUint8(0xF6);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(0, addr, OffsetFromNextInstruction);
    emitUint8(immediate.value() & 0xFF);
  } else {
    if (Ty == IceType_i16)
      emitOperandSizeOverride();
    emitUint8(0xF7);
    const uint8_t OffsetFromNextInstruction = Ty == IceType_i16 ? 2 : 4;
    emitOperand(0, addr, OffsetFromNextInstruction);
    emitImmediate(Ty, immediate);
  }
}

void AssemblerX8632::And(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<4>(Ty, dst, src);
}

void AssemblerX8632::And(Type Ty, GPRRegister dst, const AsmAddress &address) {
  arith_int<4>(Ty, dst, address);
}

void AssemblerX8632::And(Type Ty, GPRRegister dst, const Immediate &imm) {
  arith_int<4>(Ty, dst, imm);
}

void AssemblerX8632::And(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<4>(Ty, address, reg);
}

void AssemblerX8632::And(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<4>(Ty, address, imm);
}

void AssemblerX8632::Or(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<1>(Ty, dst, src);
}

void AssemblerX8632::Or(Type Ty, GPRRegister dst, const AsmAddress &address) {
  arith_int<1>(Ty, dst, address);
}

void AssemblerX8632::Or(Type Ty, GPRRegister dst, const Immediate &imm) {
  arith_int<1>(Ty, dst, imm);
}

void AssemblerX8632::Or(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<1>(Ty, address, reg);
}

void AssemblerX8632::Or(Type Ty, const AsmAddress &address,
                        const Immediate &imm) {
  arith_int<1>(Ty, address, imm);
}

void AssemblerX8632::Xor(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<6>(Ty, dst, src);
}

void AssemblerX8632::Xor(Type Ty, GPRRegister dst, const AsmAddress &address) {
  arith_int<6>(Ty, dst, address);
}

void AssemblerX8632::Xor(Type Ty, GPRRegister dst, const Immediate &imm) {
  arith_int<6>(Ty, dst, imm);
}

void AssemblerX8632::Xor(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<6>(Ty, address, reg);
}

void AssemblerX8632::Xor(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<6>(Ty, address, imm);
}

void AssemblerX8632::add(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<0>(Ty, dst, src);
}

void AssemblerX8632::add(Type Ty, GPRRegister reg, const AsmAddress &address) {
  arith_int<0>(Ty, reg, address);
}

void AssemblerX8632::add(Type Ty, GPRRegister reg, const Immediate &imm) {
  arith_int<0>(Ty, reg, imm);
}

void AssemblerX8632::add(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<0>(Ty, address, reg);
}

void AssemblerX8632::add(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<0>(Ty, address, imm);
}

void AssemblerX8632::adc(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<2>(Ty, dst, src);
}

void AssemblerX8632::adc(Type Ty, GPRRegister dst, const AsmAddress &address) {
  arith_int<2>(Ty, dst, address);
}

void AssemblerX8632::adc(Type Ty, GPRRegister reg, const Immediate &imm) {
  arith_int<2>(Ty, reg, imm);
}

void AssemblerX8632::adc(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<2>(Ty, address, reg);
}

void AssemblerX8632::adc(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<2>(Ty, address, imm);
}

void AssemblerX8632::sub(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<5>(Ty, dst, src);
}

void AssemblerX8632::sub(Type Ty, GPRRegister reg, const AsmAddress &address) {
  arith_int<5>(Ty, reg, address);
}

void AssemblerX8632::sub(Type Ty, GPRRegister reg, const Immediate &imm) {
  arith_int<5>(Ty, reg, imm);
}

void AssemblerX8632::sub(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<5>(Ty, address, reg);
}

void AssemblerX8632::sub(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<5>(Ty, address, imm);
}

void AssemblerX8632::sbb(Type Ty, GPRRegister dst, GPRRegister src) {
  arith_int<3>(Ty, dst, src);
}

void AssemblerX8632::sbb(Type Ty, GPRRegister dst, const AsmAddress &address) {
  arith_int<3>(Ty, dst, address);
}

void AssemblerX8632::sbb(Type Ty, GPRRegister reg, const Immediate &imm) {
  arith_int<3>(Ty, reg, imm);
}

void AssemblerX8632::sbb(Type Ty, const AsmAddress &address, GPRRegister reg) {
  arith_int<3>(Ty, address, reg);
}

void AssemblerX8632::sbb(Type Ty, const AsmAddress &address,
                         const Immediate &imm) {
  arith_int<3>(Ty, address, imm);
}

void AssemblerX8632::cbw() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitOperandSizeOverride();
  emitUint8(0x98);
}

void AssemblerX8632::cwd() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitOperandSizeOverride();
  emitUint8(0x99);
}

void AssemblerX8632::cdq() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x99);
}

void AssemblerX8632::div(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitRegisterOperand(6, gprEncoding(reg));
}

void AssemblerX8632::div(Type Ty, const AsmAddress &addr) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitOperand(6, addr);
}

void AssemblerX8632::idiv(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitRegisterOperand(7, gprEncoding(reg));
}

void AssemblerX8632::idiv(Type Ty, const AsmAddress &addr) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitOperand(7, addr);
}

void AssemblerX8632::imul(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xAF);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::imul(Type Ty, GPRRegister reg, const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xAF);
  emitOperand(gprEncoding(reg), address);
}

void AssemblerX8632::imul(Type Ty, GPRRegister reg, const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32 || Ty == IceType_i64);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (imm.is_int8()) {
    emitUint8(0x6B);
    emitRegisterOperand(gprEncoding(reg), gprEncoding(reg));
    emitUint8(imm.value() & 0xFF);
  } else {
    emitUint8(0x69);
    emitRegisterOperand(gprEncoding(reg), gprEncoding(reg));
    emitImmediate(Ty, imm);
  }
}

void AssemblerX8632::imul(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitRegisterOperand(5, gprEncoding(reg));
}

void AssemblerX8632::imul(Type Ty, const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitOperand(5, address);
}

void AssemblerX8632::imul(Type Ty, GPRRegister dst, GPRRegister src,
                          const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (imm.is_int8()) {
    emitUint8(0x6B);
    emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
    emitUint8(imm.value() & 0xFF);
  } else {
    emitUint8(0x69);
    emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
    emitImmediate(Ty, imm);
  }
}

void AssemblerX8632::imul(Type Ty, GPRRegister dst, const AsmAddress &address,
                          const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (imm.is_int8()) {
    emitUint8(0x6B);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(gprEncoding(dst), address, OffsetFromNextInstruction);
    emitUint8(imm.value() & 0xFF);
  } else {
    emitUint8(0x69);
    const uint8_t OffsetFromNextInstruction = Ty == IceType_i16 ? 2 : 4;
    emitOperand(gprEncoding(dst), address, OffsetFromNextInstruction);
    emitImmediate(Ty, imm);
  }
}

void AssemblerX8632::mul(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitRegisterOperand(4, gprEncoding(reg));
}

void AssemblerX8632::mul(Type Ty, const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitOperand(4, address);
}

void AssemblerX8632::incl(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x40 + reg);
}

void AssemblerX8632::incl(const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xFF);
  emitOperand(0, address);
}

void AssemblerX8632::decl(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x48 + reg);
}

void AssemblerX8632::decl(const AsmAddress &address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xFF);
  emitOperand(1, address);
}

void AssemblerX8632::rol(Type Ty, GPRRegister reg, const Immediate &imm) {
  emitGenericShift(0, Ty, reg, imm);
}

void AssemblerX8632::rol(Type Ty, GPRRegister operand, GPRRegister shifter) {
  emitGenericShift(0, Ty, AsmOperand(operand), shifter);
}

void AssemblerX8632::rol(Type Ty, const AsmAddress &operand,
                         GPRRegister shifter) {
  emitGenericShift(0, Ty, operand, shifter);
}

void AssemblerX8632::shl(Type Ty, GPRRegister reg, const Immediate &imm) {
  emitGenericShift(4, Ty, reg, imm);
}

void AssemblerX8632::shl(Type Ty, GPRRegister operand, GPRRegister shifter) {
  emitGenericShift(4, Ty, AsmOperand(operand), shifter);
}

void AssemblerX8632::shl(Type Ty, const AsmAddress &operand,
                         GPRRegister shifter) {
  emitGenericShift(4, Ty, operand, shifter);
}

void AssemblerX8632::shr(Type Ty, GPRRegister reg, const Immediate &imm) {
  emitGenericShift(5, Ty, reg, imm);
}

void AssemblerX8632::shr(Type Ty, GPRRegister operand, GPRRegister shifter) {
  emitGenericShift(5, Ty, AsmOperand(operand), shifter);
}

void AssemblerX8632::shr(Type Ty, const AsmAddress &operand,
                         GPRRegister shifter) {
  emitGenericShift(5, Ty, operand, shifter);
}

void AssemblerX8632::sar(Type Ty, GPRRegister reg, const Immediate &imm) {
  emitGenericShift(7, Ty, reg, imm);
}

void AssemblerX8632::sar(Type Ty, GPRRegister operand, GPRRegister shifter) {
  emitGenericShift(7, Ty, AsmOperand(operand), shifter);
}

void AssemblerX8632::sar(Type Ty, const AsmAddress &address,
                         GPRRegister shifter) {
  emitGenericShift(7, Ty, address, shifter);
}

void AssemblerX8632::shld(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xA5);
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
}

void AssemblerX8632::shld(Type Ty, GPRRegister dst, GPRRegister src,
                          const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  assert(imm.is_int8());
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xA4);
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
  emitUint8(imm.value() & 0xFF);
}

void AssemblerX8632::shld(Type Ty, const AsmAddress &operand, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xA5);
  emitOperand(gprEncoding(src), operand);
}

void AssemblerX8632::shrd(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xAD);
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
}

void AssemblerX8632::shrd(Type Ty, GPRRegister dst, GPRRegister src,
                          const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  assert(imm.is_int8());
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xAC);
  emitRegisterOperand(gprEncoding(src), gprEncoding(dst));
  emitUint8(imm.value() & 0xFF);
}

void AssemblerX8632::shrd(Type Ty, const AsmAddress &dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xAD);
  emitOperand(gprEncoding(src), dst);
}

void AssemblerX8632::neg(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitRegisterOperand(3, gprEncoding(reg));
}

void AssemblerX8632::neg(Type Ty, const AsmAddress &addr) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0xF6);
  else
    emitUint8(0xF7);
  emitOperand(3, addr);
}

void AssemblerX8632::notl(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF7);
  emitUint8(0xD0 | gprEncoding(reg));
}

void AssemblerX8632::bswap(Type Ty, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i32);
  emitUint8(0x0F);
  emitUint8(0xC8 | gprEncoding(reg));
}

void AssemblerX8632::bsf(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xBC);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::bsf(Type Ty, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xBC);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::bsr(Type Ty, GPRRegister dst, GPRRegister src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xBD);
  emitRegisterOperand(gprEncoding(dst), gprEncoding(src));
}

void AssemblerX8632::bsr(Type Ty, GPRRegister dst, const AsmAddress &src) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(Ty == IceType_i16 || Ty == IceType_i32);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(0x0F);
  emitUint8(0xBD);
  emitOperand(gprEncoding(dst), src);
}

void AssemblerX8632::bt(GPRRegister base, GPRRegister offset) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0xA3);
  emitRegisterOperand(gprEncoding(offset), gprEncoding(base));
}

void AssemblerX8632::ret() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xC3);
}

void AssemblerX8632::ret(const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xC2);
  assert(imm.is_uint16());
  emitUint8(imm.value() & 0xFF);
  emitUint8((imm.value() >> 8) & 0xFF);
}

void AssemblerX8632::nop(int size) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // There are nops up to size 15, but for now just provide up to size 8.
  assert(0 < size && size <= MAX_NOP_SIZE);
  switch (size) {
  case 1:
    emitUint8(0x90);
    break;
  case 2:
    emitUint8(0x66);
    emitUint8(0x90);
    break;
  case 3:
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x00);
    break;
  case 4:
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x40);
    emitUint8(0x00);
    break;
  case 5:
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x44);
    emitUint8(0x00);
    emitUint8(0x00);
    break;
  case 6:
    emitUint8(0x66);
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x44);
    emitUint8(0x00);
    emitUint8(0x00);
    break;
  case 7:
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x80);
    emitUint8(0x00);
    emitUint8(0x00);
    emitUint8(0x00);
    emitUint8(0x00);
    break;
  case 8:
    emitUint8(0x0F);
    emitUint8(0x1F);
    emitUint8(0x84);
    emitUint8(0x00);
    emitUint8(0x00);
    emitUint8(0x00);
    emitUint8(0x00);
    emitUint8(0x00);
    break;
  default:
    llvm_unreachable("Unimplemented");
  }
}

void AssemblerX8632::int3() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xCC);
}

void AssemblerX8632::hlt() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF4);
}

void AssemblerX8632::ud2() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x0B);
}

void AssemblerX8632::j(BrCond condition, Label *label, bool near) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (label->isBound()) {
    static const int kShortSize = 2;
    static const int kLongSize = 6;
    intptr_t offset = label->getPosition() - Buffer.size();
    assert(offset <= 0);
    if (Utils::IsInt(8, offset - kShortSize)) {
      emitUint8(0x70 + condition);
      emitUint8((offset - kShortSize) & 0xFF);
    } else {
      emitUint8(0x0F);
      emitUint8(0x80 + condition);
      emitInt32(offset - kLongSize);
    }
  } else if (near) {
    emitUint8(0x70 + condition);
    emitNearLabelLink(label);
  } else {
    emitUint8(0x0F);
    emitUint8(0x80 + condition);
    emitLabelLink(label);
  }
}

void AssemblerX8632::j(BrCond condition, const ConstantRelocatable *label) {
  llvm::report_fatal_error("Untested - please verify and then reenable.");
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x80 + condition);
  auto *Fixup = this->createFixup(FK_PcRel, label);
  Fixup->set_addend(-4);
  emitFixup(Fixup);
  emitInt32(0);
}

void AssemblerX8632::jmp(GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xFF);
  emitRegisterOperand(4, gprEncoding(reg));
}

void AssemblerX8632::jmp(Label *label, bool near) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (label->isBound()) {
    static const int kShortSize = 2;
    static const int kLongSize = 5;
    intptr_t offset = label->getPosition() - Buffer.size();
    assert(offset <= 0);
    if (Utils::IsInt(8, offset - kShortSize)) {
      emitUint8(0xEB);
      emitUint8((offset - kShortSize) & 0xFF);
    } else {
      emitUint8(0xE9);
      emitInt32(offset - kLongSize);
    }
  } else if (near) {
    emitUint8(0xEB);
    emitNearLabelLink(label);
  } else {
    emitUint8(0xE9);
    emitLabelLink(label);
  }
}

void AssemblerX8632::jmp(const ConstantRelocatable *label) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xE9);
  auto *Fixup = this->createFixup(FK_PcRel, label);
  Fixup->set_addend(-4);
  emitFixup(Fixup);
  emitInt32(0);
}

void AssemblerX8632::jmp(const Immediate &abs_address) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xE9);
  AssemblerFixup *Fixup = createFixup(FK_PcRel, AssemblerFixup::NullSymbol);
  Fixup->set_addend(abs_address.value() - 4);
  emitFixup(Fixup);
  emitInt32(0);
}

void AssemblerX8632::mfence() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0xAE);
  emitUint8(0xF0);
}

void AssemblerX8632::lock() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0xF0);
}

void AssemblerX8632::cmpxchg(Type Ty, const AsmAddress &address,
                             GPRRegister reg, bool Locked) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (Locked)
    emitUint8(0xF0);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty))
    emitUint8(0xB0);
  else
    emitUint8(0xB1);
  emitOperand(gprEncoding(reg), address);
}

void AssemblerX8632::cmpxchg8b(const AsmAddress &address, bool Locked) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Locked)
    emitUint8(0xF0);
  emitUint8(0x0F);
  emitUint8(0xC7);
  emitOperand(1, address);
}

void AssemblerX8632::xadd(Type Ty, const AsmAddress &addr, GPRRegister reg,
                          bool Locked) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (Locked)
    emitUint8(0xF0);
  emitUint8(0x0F);
  if (isByteSizedArithType(Ty))
    emitUint8(0xC0);
  else
    emitUint8(0xC1);
  emitOperand(gprEncoding(reg), addr);
}

void AssemblerX8632::xchg(Type Ty, GPRRegister reg0, GPRRegister reg1) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  // Use short form if either register is EAX.
  if (reg0 == RegX8632::Encoded_Reg_eax) {
    emitUint8(0x90 + gprEncoding(reg1));
  } else if (reg1 == RegX8632::Encoded_Reg_eax) {
    emitUint8(0x90 + gprEncoding(reg0));
  } else {
    if (isByteSizedArithType(Ty))
      emitUint8(0x86);
    else
      emitUint8(0x87);
    emitRegisterOperand(gprEncoding(reg0), gprEncoding(reg1));
  }
}

void AssemblerX8632::xchg(Type Ty, const AsmAddress &addr, GPRRegister reg) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (isByteSizedArithType(Ty))
    emitUint8(0x86);
  else
    emitUint8(0x87);
  emitOperand(gprEncoding(reg), addr);
}

void AssemblerX8632::iaca_start() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(0x0F);
  emitUint8(0x0B);

  // mov $111, ebx
  constexpr GPRRegister dst = GPRRegister::Encoded_Reg_ebx;
  constexpr Type Ty = IceType_i32;
  emitUint8(0xB8 + gprEncoding(dst));
  emitImmediate(Ty, Immediate(111));

  emitUint8(0x64);
  emitUint8(0x67);
  emitUint8(0x90);
}

void AssemblerX8632::iaca_end() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);

  // mov $222, ebx
  constexpr GPRRegister dst = GPRRegister::Encoded_Reg_ebx;
  constexpr Type Ty = IceType_i32;
  emitUint8(0xB8 + gprEncoding(dst));
  emitImmediate(Ty, Immediate(222));

  emitUint8(0x64);
  emitUint8(0x67);
  emitUint8(0x90);

  emitUint8(0x0F);
  emitUint8(0x0B);
}

void AssemblerX8632::emitSegmentOverride(uint8_t prefix) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  emitUint8(prefix);
}

void AssemblerX8632::align(intptr_t alignment, intptr_t offset) {
  assert(llvm::isPowerOf2_32(alignment));
  intptr_t pos = offset + Buffer.getPosition();
  intptr_t mod = pos & (alignment - 1);
  if (mod == 0) {
    return;
  }
  intptr_t bytes_needed = alignment - mod;
  while (bytes_needed > MAX_NOP_SIZE) {
    nop(MAX_NOP_SIZE);
    bytes_needed -= MAX_NOP_SIZE;
  }
  if (bytes_needed) {
    nop(bytes_needed);
  }
  assert(((offset + Buffer.getPosition()) & (alignment - 1)) == 0);
}

void AssemblerX8632::bind(Label *L) {
  const intptr_t Bound = Buffer.size();
  assert(!L->isBound()); // Labels can only be bound once.
  while (L->isLinked()) {
    const intptr_t Position = L->getLinkPosition();
    const intptr_t Next = Buffer.load<int32_t>(Position);
    const intptr_t Offset = Bound - (Position + 4);
    Buffer.store<int32_t>(Position, Offset);
    L->Position = Next;
  }
  while (L->hasNear()) {
    intptr_t Position = L->getNearPosition();
    const intptr_t Offset = Bound - (Position + 1);
    assert(Utils::IsInt(8, Offset));
    Buffer.store<int8_t>(Position, Offset);
  }
  L->bindTo(Bound);
}

void AssemblerX8632::emitOperand(int rm, const AsmOperand &operand,
                                 RelocOffsetT Addend) {
  assert(rm >= 0 && rm < 8);
  const intptr_t length = operand.length_;
  assert(length > 0);
  intptr_t displacement_start = 1;
  // Emit the ModRM byte updated with the given RM value.
  assert((operand.encoding_[0] & 0x38) == 0);
  emitUint8(operand.encoding_[0] + (rm << 3));
  // Whenever the addressing mode is not register indirect, using esp == 0x4
  // as the register operation indicates an SIB byte follows.
  if (((operand.encoding_[0] & 0xc0) != 0xc0) &&
      ((operand.encoding_[0] & 0x07) == 0x04)) {
    emitUint8(operand.encoding_[1]);
    displacement_start = 2;
  }

  AssemblerFixup *Fixup = operand.fixup();
  if (Fixup == nullptr) {
    for (intptr_t i = displacement_start; i < length; i++) {
      emitUint8(operand.encoding_[i]);
    }
    return;
  }

  // Emit the fixup, and a dummy 4-byte immediate. Note that the Disp32 in
  // operand.encoding_[i, i+1, i+2, i+3] is part of the constant relocatable
  // used to create the fixup, so there's no need to add it to the addend.
  assert(length - displacement_start == 4);
  if (fixupIsPCRel(Fixup->kind())) {
    Fixup->set_addend(Fixup->get_addend() - Addend);
  } else {
    Fixup->set_addend(Fixup->get_addend());
  }
  emitFixup(Fixup);
  emitInt32(0);
}

void AssemblerX8632::emitImmediate(Type Ty, const Immediate &imm) {
  auto *const Fixup = imm.fixup();
  if (Ty == IceType_i16) {
    assert(Fixup == nullptr);
    emitInt16(imm.value());
    return;
  }

  if (Fixup == nullptr) {
    emitInt32(imm.value());
    return;
  }

  Fixup->set_addend(Fixup->get_addend() + imm.value());
  emitFixup(Fixup);
  emitInt32(0);
}

void AssemblerX8632::emitComplexI8(int rm, const AsmOperand &operand,
                                   const Immediate &immediate) {
  assert(rm >= 0 && rm < 8);
  assert(immediate.is_int8());
  if (operand.IsRegister(RegX8632::Encoded_Reg_eax)) {
    // Use short form if the destination is al.
    emitUint8(0x04 + (rm << 3));
    emitUint8(immediate.value() & 0xFF);
  } else {
    // Use sign-extended 8-bit immediate.
    emitUint8(0x80);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(rm, operand, OffsetFromNextInstruction);
    emitUint8(immediate.value() & 0xFF);
  }
}

void AssemblerX8632::emitComplex(Type Ty, int rm, const AsmOperand &operand,
                                 const Immediate &immediate) {
  assert(rm >= 0 && rm < 8);
  if (immediate.is_int8()) {
    // Use sign-extended 8-bit immediate.
    emitUint8(0x83);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(rm, operand, OffsetFromNextInstruction);
    emitUint8(immediate.value() & 0xFF);
  } else if (operand.IsRegister(RegX8632::Encoded_Reg_eax)) {
    // Use short form if the destination is eax.
    emitUint8(0x05 + (rm << 3));
    emitImmediate(Ty, immediate);
  } else {
    emitUint8(0x81);
    const uint8_t OffsetFromNextInstruction = Ty == IceType_i16 ? 2 : 4;
    emitOperand(rm, operand, OffsetFromNextInstruction);
    emitImmediate(Ty, immediate);
  }
}

void AssemblerX8632::emitLabel(Label *label, intptr_t instruction_size) {
  if (label->isBound()) {
    intptr_t offset = label->getPosition() - Buffer.size();
    assert(offset <= 0);
    emitInt32(offset - instruction_size);
  } else {
    emitLabelLink(label);
  }
}

void AssemblerX8632::emitLabelLink(Label *Label) {
  assert(!Label->isBound());
  intptr_t Position = Buffer.size();
  emitInt32(Label->Position);
  Label->linkTo(*this, Position);
}

void AssemblerX8632::emitNearLabelLink(Label *Label) {
  assert(!Label->isBound());
  intptr_t Position = Buffer.size();
  emitUint8(0);
  Label->nearLinkTo(*this, Position);
}

void AssemblerX8632::emitGenericShift(int rm, Type Ty, GPRRegister reg,
                                      const Immediate &imm) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  // We don't assert that imm fits into 8 bits; instead, it gets masked below.
  // Note that we don't mask it further (e.g. to 5 bits) because we want the
  // same processor behavior regardless of whether it's an immediate (masked to
  // 8 bits) or in register cl (essentially ecx masked to 8 bits).
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  if (imm.value() == 1) {
    emitUint8(isByteSizedArithType(Ty) ? 0xD0 : 0xD1);
    emitOperand(rm, AsmOperand(reg));
  } else {
    emitUint8(isByteSizedArithType(Ty) ? 0xC0 : 0xC1);
    static constexpr RelocOffsetT OffsetFromNextInstruction = 1;
    emitOperand(rm, AsmOperand(reg), OffsetFromNextInstruction);
    emitUint8(imm.value() & 0xFF);
  }
}

void AssemblerX8632::emitGenericShift(int rm, Type Ty,
                                      const AsmOperand &operand,
                                      GPRRegister shifter) {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  assert(shifter == RegX8632::Encoded_Reg_ecx);
  (void)shifter;
  if (Ty == IceType_i16)
    emitOperandSizeOverride();
  emitUint8(isByteSizedArithType(Ty) ? 0xD2 : 0xD3);
  emitOperand(rm, operand);
}

} // namespace X8632
} // end of namespace Ice
