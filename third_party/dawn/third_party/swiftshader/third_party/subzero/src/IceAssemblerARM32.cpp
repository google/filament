//===- subzero/src/IceAssemblerARM32.cpp - Assembler for ARM32 --*- C++ -*-===//
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
/// \brief Implements the Assembler class for ARM32.
///
//===----------------------------------------------------------------------===//

#include "IceAssemblerARM32.h"
#include "IceCfgNode.h"
#include "IceUtils.h"

namespace {

using namespace Ice;
using namespace Ice::ARM32;

using WordType = uint32_t;
static constexpr IValueT kWordSize = sizeof(WordType);

// The following define individual bits.
static constexpr IValueT B0 = 1;
static constexpr IValueT B1 = 1 << 1;
static constexpr IValueT B2 = 1 << 2;
static constexpr IValueT B3 = 1 << 3;
static constexpr IValueT B4 = 1 << 4;
static constexpr IValueT B5 = 1 << 5;
static constexpr IValueT B6 = 1 << 6;
static constexpr IValueT B7 = 1 << 7;
static constexpr IValueT B8 = 1 << 8;
static constexpr IValueT B9 = 1 << 9;
static constexpr IValueT B10 = 1 << 10;
static constexpr IValueT B11 = 1 << 11;
static constexpr IValueT B12 = 1 << 12;
static constexpr IValueT B13 = 1 << 13;
static constexpr IValueT B14 = 1 << 14;
static constexpr IValueT B15 = 1 << 15;
static constexpr IValueT B16 = 1 << 16;
static constexpr IValueT B17 = 1 << 17;
static constexpr IValueT B18 = 1 << 18;
static constexpr IValueT B19 = 1 << 19;
static constexpr IValueT B20 = 1 << 20;
static constexpr IValueT B21 = 1 << 21;
static constexpr IValueT B22 = 1 << 22;
static constexpr IValueT B23 = 1 << 23;
static constexpr IValueT B24 = 1 << 24;
static constexpr IValueT B25 = 1 << 25;
static constexpr IValueT B26 = 1 << 26;
static constexpr IValueT B27 = 1 << 27;

// Constants used for the decoding or encoding of the individual fields of
// instructions. Based on ARM section A5.1.
static constexpr IValueT L = 1 << 20; // load (or store)
static constexpr IValueT W = 1 << 21; // writeback base register
                                      // (or leave unchanged)
static constexpr IValueT B = 1 << 22; // unsigned byte (or word)
static constexpr IValueT U = 1 << 23; // positive (or negative)
                                      // offset/index
static constexpr IValueT P = 1 << 24; // offset/pre-indexed
                                      // addressing (or
                                      // post-indexed addressing)

static constexpr IValueT kConditionShift = 28;
static constexpr IValueT kLinkShift = 24;
static constexpr IValueT kOpcodeShift = 21;
static constexpr IValueT kRdShift = 12;
static constexpr IValueT kRmShift = 0;
static constexpr IValueT kRnShift = 16;
static constexpr IValueT kRsShift = 8;
static constexpr IValueT kSShift = 20;
static constexpr IValueT kTypeShift = 25;

// Immediate instruction fields encoding.
static constexpr IValueT kImmed8Bits = 8;
static constexpr IValueT kImmed8Shift = 0;
static constexpr IValueT kRotateBits = 4;
static constexpr IValueT kRotateShift = 8;

// Shift instruction register fields encodings.
static constexpr IValueT kShiftImmShift = 7;
static constexpr IValueT kShiftImmBits = 5;
static constexpr IValueT kShiftShift = 5;
static constexpr IValueT kImmed12Bits = 12;
static constexpr IValueT kImm12Shift = 0;

// Rotation instructions (uxtb etc.).
static constexpr IValueT kRotationShift = 10;

// MemEx instructions.
static constexpr IValueT kMemExOpcodeShift = 20;

// Div instruction register field encodings.
static constexpr IValueT kDivRdShift = 16;
static constexpr IValueT kDivRmShift = 8;
static constexpr IValueT kDivRnShift = 0;

// Type of instruction encoding (bits 25-27). See ARM section A5.1
static constexpr IValueT kInstTypeDataRegister = 0;  // i.e. 000
static constexpr IValueT kInstTypeDataRegShift = 0;  // i.e. 000
static constexpr IValueT kInstTypeDataImmediate = 1; // i.e. 001
static constexpr IValueT kInstTypeMemImmediate = 2;  // i.e. 010
static constexpr IValueT kInstTypeRegisterShift = 3; // i.e. 011

// Limit on number of registers in a vpush/vpop.
static constexpr SizeT VpushVpopMaxConsecRegs = 16;

// Offset modifier to current PC for next instruction.  The offset is off by 8
// due to the way the ARM CPUs read PC.
static constexpr IOffsetT kPCReadOffset = 8;

// Mask to pull out PC offset from branch (b) instruction.
static constexpr int kBranchOffsetBits = 24;
static constexpr IOffsetT kBranchOffsetMask = 0x00ffffff;

IValueT encodeBool(bool B) { return B ? 1 : 0; }

IValueT encodeRotation(ARM32::AssemblerARM32::RotationValue Value) {
  return static_cast<IValueT>(Value);
}

IValueT encodeGPRRegister(RegARM32::GPRRegister Rn) {
  return static_cast<IValueT>(Rn);
}

RegARM32::GPRRegister decodeGPRRegister(IValueT R) {
  return static_cast<RegARM32::GPRRegister>(R);
}

IValueT encodeCondition(CondARM32::Cond Cond) {
  return static_cast<IValueT>(Cond);
}

IValueT encodeShift(OperandARM32::ShiftKind Shift) {
  // Follows encoding in ARM section A8.4.1 "Constant shifts".
  switch (Shift) {
  case OperandARM32::kNoShift:
  case OperandARM32::LSL:
    return 0; // 0b00
  case OperandARM32::LSR:
    return 1; // 0b01
  case OperandARM32::ASR:
    return 2; // 0b10
  case OperandARM32::ROR:
  case OperandARM32::RRX:
    return 3; // 0b11
  }
  llvm::report_fatal_error("Unknown Shift value");
  return 0;
}

// Returns the bits in the corresponding masked value.
IValueT mask(IValueT Value, IValueT Shift, IValueT Bits) {
  return (Value >> Shift) & ((1 << Bits) - 1);
}

// Extract out a Bit in Value.
bool isBitSet(IValueT Bit, IValueT Value) { return (Value & Bit) == Bit; }

// Returns the GPR register at given Shift in Value.
RegARM32::GPRRegister getGPRReg(IValueT Shift, IValueT Value) {
  return decodeGPRRegister((Value >> Shift) & 0xF);
}

IValueT getEncodedGPRegNum(const Variable *Var) {
  assert(Var->hasReg());
  const auto Reg = Var->getRegNum();
  return llvm::isa<Variable64On32>(Var) ? RegARM32::getI64PairFirstGPRNum(Reg)
                                        : RegARM32::getEncodedGPR(Reg);
}

IValueT getEncodedSRegNum(const Variable *Var) {
  assert(Var->hasReg());
  return RegARM32::getEncodedSReg(Var->getRegNum());
}

IValueT getEncodedDRegNum(const Variable *Var) {
  return RegARM32::getEncodedDReg(Var->getRegNum());
}

IValueT getEncodedQRegNum(const Variable *Var) {
  return RegARM32::getEncodedQReg(Var->getRegNum());
}

IValueT mapQRegToDReg(IValueT EncodedQReg) {
  IValueT DReg = EncodedQReg << 1;
  assert(DReg < RegARM32::getNumDRegs());
  return DReg;
}

IValueT mapQRegToSReg(IValueT EncodedQReg) {
  IValueT SReg = EncodedQReg << 2;
  assert(SReg < RegARM32::getNumSRegs());
  return SReg;
}

IValueT getYInRegXXXXY(IValueT RegXXXXY) { return RegXXXXY & 0x1; }

IValueT getXXXXInRegXXXXY(IValueT RegXXXXY) { return RegXXXXY >> 1; }

IValueT getYInRegYXXXX(IValueT RegYXXXX) { return RegYXXXX >> 4; }

IValueT getXXXXInRegYXXXX(IValueT RegYXXXX) { return RegYXXXX & 0x0f; }

// Figures out Op/Cmode values for given Value. Returns true if able to encode.
bool encodeAdvSIMDExpandImm(IValueT Value, Type ElmtTy, IValueT &Op,
                            IValueT &Cmode, IValueT &Imm8) {
  // TODO(kschimpf): Handle other shifted 8-bit values.
  constexpr IValueT Imm8Mask = 0xFF;
  if ((Value & IValueT(~Imm8Mask)) != 0)
    return false;
  Imm8 = Value;
  switch (ElmtTy) {
  case IceType_i8:
    Op = 0;
    Cmode = 14; // 0b1110
    return true;
  case IceType_i16:
    Op = 0;
    Cmode = 8; // 0b1000
    return true;
  case IceType_i32:
    Op = 0;
    Cmode = 0; // 0b0000
    return true;
  default:
    return false;
  }
}

// Defines layouts of an operand representing a (register) memory address,
// possibly modified by an immediate value.
enum EncodedImmAddress {
  // Address modified by a rotated immediate 8-bit value.
  RotatedImm8Address,

  // Alternate encoding for RotatedImm8Address, where the offset is divided by 4
  // before encoding.
  RotatedImm8Div4Address,

  // Address modified by an immediate 12-bit value.
  Imm12Address,

  // Alternate encoding 3, for an address modified by a rotated immediate 8-bit
  // value.
  RotatedImm8Enc3Address,

  // Encoding where no immediate offset is used.
  NoImmOffsetAddress
};

// The way an operand is encoded into a sequence of bits in functions
// encodeOperand and encodeAddress below.
enum EncodedOperand {
  // Unable to encode, value left undefined.
  CantEncode = 0,

  // Value is register found.
  EncodedAsRegister,

  // Value=rrrriiiiiiii where rrrr is the rotation, and iiiiiiii is the imm8
  // value.
  EncodedAsRotatedImm8,

  // EncodedAsImmRegOffset is a memory operand that can take three forms, based
  // on type EncodedImmAddress:
  //
  // ***** RotatedImm8Address *****
  //
  // Value=0000000pu0w0nnnn0000iiiiiiiiiiii where nnnn is the base register Rn,
  // p=1 if pre-indexed addressing, u=1 if offset positive, w=1 if writeback to
  // Rn should be used, and iiiiiiiiiiii defines the rotated Imm8 value.
  //
  // ***** RotatedImm8Div4Address *****
  //
  // Value=00000000pu0w0nnnn0000iiii0000jjjj where nnnn=Rn, iiiijjjj=Imm8, p=1
  // if pre-indexed addressing, u=1 if offset positive, and w=1 if writeback to
  // Rn.
  //
  // ***** Imm12Address *****
  //
  // Value=0000000pu0w0nnnn0000iiiiiiiiiiii where nnnn is the base register Rn,
  // p=1 if pre-indexed addressing, u=1 if offset positive, w=1 if writeback to
  // Rn should be used, and iiiiiiiiiiii defines the immediate 12-bit value.
  //
  // ***** NoImmOffsetAddress *****
  //
  // Value=000000001000nnnn0000000000000000 where nnnn=Rn.
  EncodedAsImmRegOffset,

  // Value=0000000pu0w00nnnnttttiiiiiss0mmmm where nnnn is the base register Rn,
  // mmmm is the index register Rm, iiiii is the shift amount, ss is the shift
  // kind, p=1 if pre-indexed addressing, u=1 if offset positive, and w=1 if
  // writeback to Rn.
  EncodedAsShiftRotateImm5,

  // Value=000000000000000000000iiiii0000000 where iiii defines the Imm5 value
  // to shift.
  EncodedAsShiftImm5,

  // Value=iiiiiss0mmmm where mmmm is the register to rotate, ss is the shift
  // kind, and iiiii is the shift amount.
  EncodedAsShiftedRegister,

  // Value=ssss0tt1mmmm where mmmm=Rm, tt is an encoded ShiftKind, and ssss=Rms.
  EncodedAsRegShiftReg,

  // Value is 32bit integer constant.
  EncodedAsConstI32
};

// Sets Encoding to a rotated Imm8 encoding of Value, if possible.
IValueT encodeRotatedImm8(IValueT RotateAmt, IValueT Immed8) {
  assert(RotateAmt < (1 << kRotateBits));
  assert(Immed8 < (1 << kImmed8Bits));
  return (RotateAmt << kRotateShift) | (Immed8 << kImmed8Shift);
}

// Encodes iiiiitt0mmmm for data-processing (2nd) operands where iiiii=Imm5,
// tt=Shift, and mmmm=Rm.
IValueT encodeShiftRotateImm5(IValueT Rm, OperandARM32::ShiftKind Shift,
                              IOffsetT imm5) {
  (void)kShiftImmBits;
  assert(imm5 < (1 << kShiftImmBits));
  return (imm5 << kShiftImmShift) | (encodeShift(Shift) << kShiftShift) | Rm;
}

// Encodes mmmmtt01ssss for data-processing operands where mmmm=Rm, ssss=Rs, and
// tt=Shift.
IValueT encodeShiftRotateReg(IValueT Rm, OperandARM32::ShiftKind Shift,
                             IValueT Rs) {
  return (Rs << kRsShift) | (encodeShift(Shift) << kShiftShift) | B4 |
         (Rm << kRmShift);
}

// Defines the set of registers expected in an operand.
enum RegSetWanted { WantGPRegs, WantSRegs, WantDRegs, WantQRegs };

EncodedOperand encodeOperand(const Operand *Opnd, IValueT &Value,
                             RegSetWanted WantedRegSet) {
  Value = 0; // Make sure initialized.
  if (const auto *Var = llvm::dyn_cast<Variable>(Opnd)) {
    if (Var->hasReg()) {
      switch (WantedRegSet) {
      case WantGPRegs:
        Value = getEncodedGPRegNum(Var);
        break;
      case WantSRegs:
        Value = getEncodedSRegNum(Var);
        break;
      case WantDRegs:
        Value = getEncodedDRegNum(Var);
        break;
      case WantQRegs:
        Value = getEncodedQRegNum(Var);
        break;
      }
      return EncodedAsRegister;
    }
    return CantEncode;
  }
  if (const auto *FlexImm = llvm::dyn_cast<OperandARM32FlexImm>(Opnd)) {
    const IValueT Immed8 = FlexImm->getImm();
    const IValueT Rotate = FlexImm->getRotateAmt();
    if (!((Rotate < (1 << kRotateBits)) && (Immed8 < (1 << kImmed8Bits))))
      return CantEncode;
    Value = (Rotate << kRotateShift) | (Immed8 << kImmed8Shift);
    return EncodedAsRotatedImm8;
  }
  if (const auto *Const = llvm::dyn_cast<ConstantInteger32>(Opnd)) {
    Value = Const->getValue();
    return EncodedAsConstI32;
  }
  if (const auto *FlexReg = llvm::dyn_cast<OperandARM32FlexReg>(Opnd)) {
    Operand *Amt = FlexReg->getShiftAmt();
    IValueT Rm;
    if (encodeOperand(FlexReg->getReg(), Rm, WantGPRegs) != EncodedAsRegister)
      return CantEncode;
    if (const auto *Var = llvm::dyn_cast<Variable>(Amt)) {
      IValueT Rs;
      if (encodeOperand(Var, Rs, WantGPRegs) != EncodedAsRegister)
        return CantEncode;
      Value = encodeShiftRotateReg(Rm, FlexReg->getShiftOp(), Rs);
      return EncodedAsRegShiftReg;
    }
    // If reached, the amount is a shifted amount by some 5-bit immediate.
    uint32_t Imm5;
    if (const auto *ShAmt = llvm::dyn_cast<OperandARM32ShAmtImm>(Amt)) {
      Imm5 = ShAmt->getShAmtImm();
    } else if (const auto *IntConst = llvm::dyn_cast<ConstantInteger32>(Amt)) {
      int32_t Val = IntConst->getValue();
      if (Val < 0)
        return CantEncode;
      Imm5 = static_cast<uint32_t>(Val);
    } else
      return CantEncode;
    Value = encodeShiftRotateImm5(Rm, FlexReg->getShiftOp(), Imm5);
    return EncodedAsShiftedRegister;
  }
  if (const auto *ShImm = llvm::dyn_cast<OperandARM32ShAmtImm>(Opnd)) {
    const IValueT Immed5 = ShImm->getShAmtImm();
    assert(Immed5 < (1 << kShiftImmBits));
    Value = (Immed5 << kShiftImmShift);
    return EncodedAsShiftImm5;
  }
  return CantEncode;
}

IValueT encodeImmRegOffset(IValueT Reg, IOffsetT Offset,
                           OperandARM32Mem::AddrMode Mode, IOffsetT MaxOffset,
                           IValueT OffsetShift) {
  IValueT Value = Mode | (Reg << kRnShift);
  if (Offset < 0) {
    Offset = -Offset;
    Value ^= U; // Flip U to adjust sign.
  }
  assert(Offset <= MaxOffset);
  (void)MaxOffset;
  return Value | (Offset >> OffsetShift);
}

// Encodes immediate register offset using encoding 3.
IValueT encodeImmRegOffsetEnc3(IValueT Rn, IOffsetT Imm8,
                               OperandARM32Mem::AddrMode Mode) {
  IValueT Value = Mode | (Rn << kRnShift);
  if (Imm8 < 0) {
    Imm8 = -Imm8;
    Value = (Value ^ U);
  }
  assert(Imm8 < (1 << 8));
  Value = Value | B22 | ((Imm8 & 0xf0) << 4) | (Imm8 & 0x0f);
  return Value;
}

IValueT encodeImmRegOffset(EncodedImmAddress ImmEncoding, IValueT Reg,
                           IOffsetT Offset, OperandARM32Mem::AddrMode Mode) {
  switch (ImmEncoding) {
  case RotatedImm8Address: {
    constexpr IOffsetT MaxOffset = (1 << 8) - 1;
    constexpr IValueT NoRightShift = 0;
    return encodeImmRegOffset(Reg, Offset, Mode, MaxOffset, NoRightShift);
  }
  case RotatedImm8Div4Address: {
    assert((Offset & 0x3) == 0);
    constexpr IOffsetT MaxOffset = (1 << 8) - 1;
    constexpr IValueT RightShift2 = 2;
    return encodeImmRegOffset(Reg, Offset, Mode, MaxOffset, RightShift2);
  }
  case Imm12Address: {
    constexpr IOffsetT MaxOffset = (1 << 12) - 1;
    constexpr IValueT NoRightShift = 0;
    return encodeImmRegOffset(Reg, Offset, Mode, MaxOffset, NoRightShift);
  }
  case RotatedImm8Enc3Address:
    return encodeImmRegOffsetEnc3(Reg, Offset, Mode);
  case NoImmOffsetAddress: {
    assert(Offset == 0);
    assert(Mode == OperandARM32Mem::Offset);
    return Reg << kRnShift;
  }
  }
  llvm_unreachable("(silence g++ warning)");
}

// Encodes memory address Opnd, and encodes that information into Value, based
// on how ARM represents the address. Returns how the value was encoded.
EncodedOperand encodeAddress(const Operand *Opnd, IValueT &Value,
                             const AssemblerARM32::TargetInfo &TInfo,
                             EncodedImmAddress ImmEncoding) {
  Value = 0; // Make sure initialized.
  if (const auto *Var = llvm::dyn_cast<Variable>(Opnd)) {
    // Should be a stack variable, with an offset.
    if (Var->hasReg())
      return CantEncode;
    IOffsetT Offset = Var->getStackOffset();
    if (!Utils::IsAbsoluteUint(12, Offset))
      return CantEncode;
    const auto BaseRegNum =
        Var->hasReg() ? Var->getBaseRegNum() : TInfo.FrameOrStackReg;
    Value = encodeImmRegOffset(ImmEncoding, BaseRegNum, Offset,
                               OperandARM32Mem::Offset);
    return EncodedAsImmRegOffset;
  }
  if (const auto *Mem = llvm::dyn_cast<OperandARM32Mem>(Opnd)) {
    Variable *Var = Mem->getBase();
    if (!Var->hasReg())
      return CantEncode;
    IValueT Rn = getEncodedGPRegNum(Var);
    if (Mem->isRegReg()) {
      const Variable *Index = Mem->getIndex();
      if (Var == nullptr)
        return CantEncode;
      Value = (Rn << kRnShift) | Mem->getAddrMode() |
              encodeShiftRotateImm5(getEncodedGPRegNum(Index),
                                    Mem->getShiftOp(), Mem->getShiftAmt());
      return EncodedAsShiftRotateImm5;
    }
    // Encoded as immediate register offset.
    ConstantInteger32 *Offset = Mem->getOffset();
    Value = encodeImmRegOffset(ImmEncoding, Rn, Offset->getValue(),
                               Mem->getAddrMode());
    return EncodedAsImmRegOffset;
  }
  return CantEncode;
}

// Checks that Offset can fit in imm24 constant of branch (b) instruction.
void assertCanEncodeBranchOffset(IOffsetT Offset) {
  (void)Offset;
  (void)kBranchOffsetBits;
  assert(Utils::IsAligned(Offset, 4) &&
         Utils::IsInt(kBranchOffsetBits, Offset >> 2));
}

IValueT encodeBranchOffset(IOffsetT Offset, IValueT Inst) {
  // Adjust offset to the way ARM CPUs read PC.
  Offset -= kPCReadOffset;

  assertCanEncodeBranchOffset(Offset);

  // Properly preserve only the bits supported in the instruction.
  Offset >>= 2;
  Offset &= kBranchOffsetMask;
  return (Inst & ~kBranchOffsetMask) | Offset;
}

IValueT encodeRegister(const Operand *OpReg, RegSetWanted WantedRegSet,
                       const char *RegName, const char *InstName) {
  IValueT Reg = 0;
  if (encodeOperand(OpReg, Reg, WantedRegSet) != EncodedAsRegister)
    llvm::report_fatal_error(std::string(InstName) + ": Can't find register " +
                             RegName);
  return Reg;
}

IValueT encodeGPRegister(const Operand *OpReg, const char *RegName,
                         const char *InstName) {
  return encodeRegister(OpReg, WantGPRegs, RegName, InstName);
}

IValueT encodeSRegister(const Operand *OpReg, const char *RegName,
                        const char *InstName) {
  return encodeRegister(OpReg, WantSRegs, RegName, InstName);
}

IValueT encodeDRegister(const Operand *OpReg, const char *RegName,
                        const char *InstName) {
  return encodeRegister(OpReg, WantDRegs, RegName, InstName);
}

IValueT encodeQRegister(const Operand *OpReg, const char *RegName,
                        const char *InstName) {
  return encodeRegister(OpReg, WantQRegs, RegName, InstName);
}

void verifyPOrNotW(IValueT Address, const char *InstName) {
  if (BuildDefs::minimal())
    return;
  if (!isBitSet(P, Address) && isBitSet(W, Address))
    llvm::report_fatal_error(std::string(InstName) +
                             ": P=0 when W=1 not allowed");
}

void verifyRegsNotEq(IValueT Reg1, const char *Reg1Name, IValueT Reg2,
                     const char *Reg2Name, const char *InstName) {
  if (BuildDefs::minimal())
    return;
  if (Reg1 == Reg2)
    llvm::report_fatal_error(std::string(InstName) + ": " + Reg1Name + "=" +
                             Reg2Name + " not allowed");
}

void verifyRegNotPc(IValueT Reg, const char *RegName, const char *InstName) {
  verifyRegsNotEq(Reg, RegName, RegARM32::Encoded_Reg_pc, "pc", InstName);
}

void verifyAddrRegNotPc(IValueT RegShift, IValueT Address, const char *RegName,
                        const char *InstName) {
  if (BuildDefs::minimal())
    return;
  if (getGPRReg(RegShift, Address) == RegARM32::Encoded_Reg_pc)
    llvm::report_fatal_error(std::string(InstName) + ": " + RegName +
                             "=pc not allowed");
}

void verifyRegNotPcWhenSetFlags(IValueT Reg, bool SetFlags,
                                const char *InstName) {
  if (BuildDefs::minimal())
    return;
  if (SetFlags && (Reg == RegARM32::Encoded_Reg_pc))
    llvm::report_fatal_error(std::string(InstName) + ": " +
                             RegARM32::getRegName(RegARM32::Reg_pc) +
                             "=pc not allowed when CC=1");
}

enum SIMDShiftType { ST_Vshl, ST_Vshr };

IValueT encodeSIMDShiftImm6(SIMDShiftType Shift, Type ElmtTy,
                            const IValueT Imm) {
  assert(Imm > 0);
  const SizeT MaxShift = getScalarIntBitWidth(ElmtTy);
  assert(Imm < 2 * MaxShift);
  assert(ElmtTy == IceType_i8 || ElmtTy == IceType_i16 ||
         ElmtTy == IceType_i32);
  const IValueT VshlImm = Imm - MaxShift;
  const IValueT VshrImm = 2 * MaxShift - Imm;
  return ((Shift == ST_Vshl) ? VshlImm : VshrImm) & (2 * MaxShift - 1);
}

IValueT encodeSIMDShiftImm6(SIMDShiftType Shift, Type ElmtTy,
                            const ConstantInteger32 *Imm6) {
  const IValueT Imm = Imm6->getValue();
  return encodeSIMDShiftImm6(Shift, ElmtTy, Imm);
}
} // end of anonymous namespace

namespace Ice {
namespace ARM32 {

size_t MoveRelocatableFixup::emit(GlobalContext *Ctx,
                                  const Assembler &Asm) const {
  if (!BuildDefs::dump())
    return InstARM32::InstSize;
  Ostream &Str = Ctx->getStrEmit();
  IValueT Inst = Asm.load<IValueT>(position());
  const bool IsMovw = kind() == llvm::ELF::R_ARM_MOVW_ABS_NC ||
                      kind() == llvm::ELF::R_ARM_MOVW_PREL_NC;
  const auto Symbol = symbol().toString();
  const bool NeedsPCRelSuffix =
      (Asm.fixupIsPCRel(kind()) || Symbol == GlobalOffsetTable);
  Str << "\t"
         "mov"
      << (IsMovw ? "w" : "t") << "\t"
      << RegARM32::getRegName(RegNumT::fixme((Inst >> kRdShift) & 0xF))
      << ", #:" << (IsMovw ? "lower" : "upper") << "16:" << Symbol
      << (NeedsPCRelSuffix ? " - ." : "")
      << "\t@ .word "
      // TODO(jpp): This is broken, it also needs to add a magic constant.
      << llvm::format_hex_no_prefix(Inst, 8) << "\n";
  return InstARM32::InstSize;
}

IValueT AssemblerARM32::encodeElmtType(Type ElmtTy) {
  switch (ElmtTy) {
  case IceType_i8:
    return 0;
  case IceType_i16:
    return 1;
  case IceType_i32:
  case IceType_f32:
    return 2;
  case IceType_i64:
    return 3;
  default:
    llvm::report_fatal_error("SIMD op: Don't understand element type " +
                             typeStdString(ElmtTy));
  }
}

// This fixup points to an ARM32 instruction with the following format:
void MoveRelocatableFixup::emitOffset(Assembler *Asm) const {
  // cccc00110T00iiiiddddiiiiiiiiiiii where cccc=Cond, dddd=Rd,
  // iiiiiiiiiiiiiiii = Imm16, and T=1 for movt.

  const IValueT Inst = Asm->load<IValueT>(position());
  constexpr IValueT Imm16Mask = 0x000F0FFF;
  const IValueT Imm16 = offset() & 0xffff;
  Asm->store(position(),
             (Inst & ~Imm16Mask) | ((Imm16 >> 12) << 16) | (Imm16 & 0xfff));
}

MoveRelocatableFixup *AssemblerARM32::createMoveFixup(bool IsMovW,
                                                      const Constant *Value) {
  MoveRelocatableFixup *F =
      new (allocate<MoveRelocatableFixup>()) MoveRelocatableFixup();
  F->set_kind(IsMovW ? llvm::ELF::R_ARM_MOVW_ABS_NC
                     : llvm::ELF::R_ARM_MOVT_ABS);
  F->set_value(Value);
  Buffer.installFixup(F);
  return F;
}

size_t BlRelocatableFixup::emit(GlobalContext *Ctx,
                                const Assembler &Asm) const {
  if (!BuildDefs::dump())
    return InstARM32::InstSize;
  Ostream &Str = Ctx->getStrEmit();
  IValueT Inst = Asm.load<IValueT>(position());
  Str << "\t"
         "bl\t"
      << symbol() << "\t@ .word " << llvm::format_hex_no_prefix(Inst, 8)
      << "\n";
  return InstARM32::InstSize;
}

void BlRelocatableFixup::emitOffset(Assembler *Asm) const {
  // cccc101liiiiiiiiiiiiiiiiiiiiiiii where cccc=Cond, l=Link, and
  // iiiiiiiiiiiiiiiiiiiiiiii=
  // EncodedBranchOffset(cccc101l000000000000000000000000, Offset);
  const IValueT Inst = Asm->load<IValueT>(position());
  constexpr IValueT OffsetMask = 0x00FFFFFF;
  Asm->store(position(), encodeBranchOffset(offset(), Inst & ~OffsetMask));
}

void AssemblerARM32::padWithNop(intptr_t Padding) {
  constexpr intptr_t InstWidth = sizeof(IValueT);
  assert(Padding % InstWidth == 0 &&
         "Padding not multiple of instruction size");
  for (intptr_t i = 0; i < Padding; i += InstWidth)
    nop();
}

BlRelocatableFixup *
AssemblerARM32::createBlFixup(const ConstantRelocatable *BlTarget) {
  BlRelocatableFixup *F =
      new (allocate<BlRelocatableFixup>()) BlRelocatableFixup();
  F->set_kind(llvm::ELF::R_ARM_CALL);
  F->set_value(BlTarget);
  Buffer.installFixup(F);
  return F;
}

void AssemblerARM32::bindCfgNodeLabel(const CfgNode *Node) {
  if (BuildDefs::dump() && !getFlags().getDisableHybridAssembly()) {
    // Generate label name so that branches can find it.
    constexpr SizeT InstSize = 0;
    emitTextInst(Node->getAsmName() + ":", InstSize);
  }
  SizeT NodeNumber = Node->getIndex();
  assert(!getPreliminary());
  Label *L = getOrCreateCfgNodeLabel(NodeNumber);
  this->bind(L);
}

Label *AssemblerARM32::getOrCreateLabel(SizeT Number, LabelVector &Labels) {
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
  if (!L) {
    L = new (this->allocate<Label>()) Label();
    Labels[Number] = L;
  }
  return L;
}

// Pull out offset from branch Inst.
IOffsetT AssemblerARM32::decodeBranchOffset(IValueT Inst) {
  // Sign-extend, left-shift by 2, and adjust to the way ARM CPUs read PC.
  const IOffsetT Offset = (Inst & kBranchOffsetMask) << 8;
  return (Offset >> 6) + kPCReadOffset;
}

void AssemblerARM32::bind(Label *L) {
  IOffsetT BoundPc = Buffer.size();
  assert(!L->isBound()); // Labels can only be bound once.
  while (L->isLinked()) {
    IOffsetT Position = L->getLinkPosition();
    IOffsetT Dest = BoundPc - Position;
    IValueT Inst = Buffer.load<IValueT>(Position);
    Buffer.store<IValueT>(Position, encodeBranchOffset(Dest, Inst));
    L->setPosition(decodeBranchOffset(Inst));
  }
  L->bindTo(BoundPc);
}

void AssemblerARM32::emitTextInst(const std::string &Text, SizeT InstSize) {
  AssemblerFixup *F = createTextFixup(Text, InstSize);
  emitFixup(F);
  for (SizeT I = 0; I < InstSize; ++I) {
    AssemblerBuffer::EnsureCapacity ensured(&Buffer);
    Buffer.emit<char>(0);
  }
}

void AssemblerARM32::emitType01(CondARM32::Cond Cond, IValueT InstType,
                                IValueT Opcode, bool SetFlags, IValueT Rn,
                                IValueT Rd, IValueT Imm12,
                                EmitChecks RuleChecks, const char *InstName) {
  switch (RuleChecks) {
  case NoChecks:
    break;
  case RdIsPcAndSetFlags:
    verifyRegNotPcWhenSetFlags(Rd, SetFlags, InstName);
    break;
  }
  assert(Rd < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) |
                           (InstType << kTypeShift) | (Opcode << kOpcodeShift) |
                           (encodeBool(SetFlags) << kSShift) |
                           (Rn << kRnShift) | (Rd << kRdShift) | Imm12;
  emitInst(Encoding);
}

void AssemblerARM32::emitType01(CondARM32::Cond Cond, IValueT Opcode,
                                const Operand *OpRd, const Operand *OpRn,
                                const Operand *OpSrc1, bool SetFlags,
                                EmitChecks RuleChecks, const char *InstName) {
  IValueT Rd = encodeGPRegister(OpRd, "Rd", InstName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", InstName);
  emitType01(Cond, Opcode, Rd, Rn, OpSrc1, SetFlags, RuleChecks, InstName);
}

void AssemblerARM32::emitType01(CondARM32::Cond Cond, IValueT Opcode,
                                IValueT Rd, IValueT Rn, const Operand *OpSrc1,
                                bool SetFlags, EmitChecks RuleChecks,
                                const char *InstName) {
  IValueT Src1Value;
  // TODO(kschimpf) Other possible decodings of data operations.
  switch (encodeOperand(OpSrc1, Src1Value, WantGPRegs)) {
  default:
    llvm::report_fatal_error(std::string(InstName) +
                             ": Can't encode instruction");
    return;
  case EncodedAsRegister: {
    // XXX (register)
    //   xxx{s}<c> <Rd>, <Rn>, <Rm>{, <shiff>}
    //
    // cccc000xxxxsnnnnddddiiiiitt0mmmm where cccc=Cond, xxxx=Opcode, dddd=Rd,
    // nnnn=Rn, mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
    constexpr IValueT Imm5 = 0;
    Src1Value = encodeShiftRotateImm5(Src1Value, OperandARM32::kNoShift, Imm5);
    emitType01(Cond, kInstTypeDataRegister, Opcode, SetFlags, Rn, Rd, Src1Value,
               RuleChecks, InstName);
    return;
  }
  case EncodedAsShiftedRegister: {
    // Form is defined in case EncodedAsRegister. (i.e. XXX (register)).
    emitType01(Cond, kInstTypeDataRegister, Opcode, SetFlags, Rn, Rd, Src1Value,
               RuleChecks, InstName);
    return;
  }
  case EncodedAsConstI32: {
    // See if we can convert this to an XXX (immediate).
    IValueT RotateAmt;
    IValueT Imm8;
    if (!OperandARM32FlexImm::canHoldImm(Src1Value, &RotateAmt, &Imm8))
      llvm::report_fatal_error(std::string(InstName) +
                               ": Immediate rotated constant not valid");
    Src1Value = encodeRotatedImm8(RotateAmt, Imm8);
    // Intentionally fall to next case!
  }
  case EncodedAsRotatedImm8: {
    // XXX (Immediate)
    //   xxx{s}<c> <Rd>, <Rn>, #<RotatedImm8>
    //
    // cccc001xxxxsnnnnddddiiiiiiiiiiii where cccc=Cond, xxxx=Opcode, dddd=Rd,
    // nnnn=Rn, s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
    emitType01(Cond, kInstTypeDataImmediate, Opcode, SetFlags, Rn, Rd,
               Src1Value, RuleChecks, InstName);
    return;
  }
  case EncodedAsRegShiftReg: {
    // XXX (register-shifted reg)
    //   xxx{s}<c> <Rd>, <Rn>, <Rm>, <type> <Rs>
    //
    // cccc000xxxxfnnnnddddssss0tt1mmmm where cccc=Cond, xxxx=Opcode, dddd=Rd,
    // nnnn=Rn, ssss=Rs, f=SetFlags, tt is encoding of type, and
    // Src1Value=ssss01tt1mmmm.
    emitType01(Cond, kInstTypeDataRegShift, Opcode, SetFlags, Rn, Rd, Src1Value,
               RuleChecks, InstName);
    return;
  }
  }
}

void AssemblerARM32::emitType05(CondARM32::Cond Cond, IOffsetT Offset,
                                bool Link) {
  // cccc101liiiiiiiiiiiiiiiiiiiiiiii where cccc=Cond, l=Link, and
  // iiiiiiiiiiiiiiiiiiiiiiii=
  // EncodedBranchOffset(cccc101l000000000000000000000000, Offset);
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = static_cast<int32_t>(Cond) << kConditionShift |
                     5 << kTypeShift | (Link ? 1 : 0) << kLinkShift;
  Encoding = encodeBranchOffset(Offset, Encoding);
  emitInst(Encoding);
}

void AssemblerARM32::emitBranch(Label *L, CondARM32::Cond Cond, bool Link) {
  // TODO(kschimpf): Handle far jumps.
  if (L->isBound()) {
    const int32_t Dest = L->getPosition() - Buffer.size();
    emitType05(Cond, Dest, Link);
    return;
  }
  const IOffsetT Position = Buffer.size();
  // Use the offset field of the branch instruction for linking the sites.
  emitType05(Cond, L->getEncodedPosition(), Link);
  L->linkTo(*this, Position);
}

void AssemblerARM32::emitCompareOp(CondARM32::Cond Cond, IValueT Opcode,
                                   const Operand *OpRn, const Operand *OpSrc1,
                                   const char *InstName) {
  // XXX (register)
  //   XXX<c> <Rn>, <Rm>{, <shift>}
  //
  // ccccyyyxxxx1nnnn0000iiiiitt0mmmm where cccc=Cond, nnnn=Rn, mmmm=Rm, iiiii
  // defines shift constant, tt=ShiftKind, yyy=kInstTypeDataRegister, and
  // xxxx=Opcode.
  //
  // XXX (immediate)
  //  XXX<c> <Rn>, #<RotatedImm8>
  //
  // ccccyyyxxxx1nnnn0000iiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // yyy=kInstTypeDataImmdiate, xxxx=Opcode, and iiiiiiiiiiii=Src1Value
  // defining RotatedImm8.
  constexpr bool SetFlags = true;
  constexpr IValueT Rd = RegARM32::Encoded_Reg_r0;
  IValueT Rn = encodeGPRegister(OpRn, "Rn", InstName);
  emitType01(Cond, Opcode, Rd, Rn, OpSrc1, SetFlags, NoChecks, InstName);
}

void AssemblerARM32::emitMemOp(CondARM32::Cond Cond, IValueT InstType,
                               bool IsLoad, bool IsByte, IValueT Rt,
                               IValueT Address) {
  assert(Rt < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) |
                           (InstType << kTypeShift) | (IsLoad ? L : 0) |
                           (IsByte ? B : 0) | (Rt << kRdShift) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::emitMemOp(CondARM32::Cond Cond, bool IsLoad, bool IsByte,
                               IValueT Rt, const Operand *OpAddress,
                               const TargetInfo &TInfo, const char *InstName) {
  IValueT Address;
  switch (encodeAddress(OpAddress, Address, TInfo, Imm12Address)) {
  default:
    llvm::report_fatal_error(std::string(InstName) +
                             ": Memory address not understood");
  case EncodedAsImmRegOffset: {
    // XXX{B} (immediate):
    //   xxx{b}<c> <Rt>, [<Rn>{, #+/-<imm12>}]      ; p=1, w=0
    //   xxx{b}<c> <Rt>, [<Rn>], #+/-<imm12>        ; p=1, w=1
    //   xxx{b}<c> <Rt>, [<Rn>, #+/-<imm12>]!       ; p=0, w=1
    //
    // cccc010pubwlnnnnttttiiiiiiiiiiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiiiiiii=imm12, b=IsByte, pu0w<<21 is a BlockAddr, l=IsLoad, and
    // pu0w0nnnn0000iiiiiiiiiiii=Address.
    RegARM32::GPRRegister Rn = getGPRReg(kRnShift, Address);

    // Check if conditions of rules violated.
    verifyRegNotPc(Rn, "Rn", InstName);
    verifyPOrNotW(Address, InstName);
    if (!IsByte && (Rn == RegARM32::Encoded_Reg_sp) && !isBitSet(P, Address) &&
        isBitSet(U, Address) && !isBitSet(W, Address) &&
        (mask(Address, kImm12Shift, kImmed12Bits) == 0x8 /* 000000000100 */))
      llvm::report_fatal_error(std::string(InstName) +
                               ": Use push/pop instead");

    emitMemOp(Cond, kInstTypeMemImmediate, IsLoad, IsByte, Rt, Address);
    return;
  }
  case EncodedAsShiftRotateImm5: {
    // XXX{B} (register)
    //   xxx{b}<c> <Rt>, [<Rn>, +/-<Rm>{, <shift>}]{!}
    //   xxx{b}<c> <Rt>, [<Rn>], +/-<Rm>{, <shift>}
    //
    // cccc011pubwlnnnnttttiiiiiss0mmmm where cccc=Cond, tttt=Rt,
    // b=IsByte, U=1 if +, pu0b is a BlockAddr, l=IsLoad, and
    // pu0w0nnnn0000iiiiiss0mmmm=Address.
    RegARM32::GPRRegister Rn = getGPRReg(kRnShift, Address);
    RegARM32::GPRRegister Rm = getGPRReg(kRmShift, Address);

    // Check if conditions of rules violated.
    verifyPOrNotW(Address, InstName);
    verifyRegNotPc(Rm, "Rm", InstName);
    if (IsByte)
      verifyRegNotPc(Rt, "Rt", InstName);
    if (isBitSet(W, Address)) {
      verifyRegNotPc(Rn, "Rn", InstName);
      verifyRegsNotEq(Rn, "Rn", Rt, "Rt", InstName);
    }
    emitMemOp(Cond, kInstTypeRegisterShift, IsLoad, IsByte, Rt, Address);
    return;
  }
  }
}

void AssemblerARM32::emitMemOpEnc3(CondARM32::Cond Cond, IValueT Opcode,
                                   IValueT Rt, const Operand *OpAddress,
                                   const TargetInfo &TInfo,
                                   const char *InstName) {
  IValueT Address;
  switch (encodeAddress(OpAddress, Address, TInfo, RotatedImm8Enc3Address)) {
  default:
    llvm::report_fatal_error(std::string(InstName) +
                             ": Memory address not understood");
  case EncodedAsImmRegOffset: {
    // XXXH (immediate)
    //   xxxh<c> <Rt>, [<Rn>{, #+-<Imm8>}]
    //   xxxh<c> <Rt>, [<Rn>, #+/-<Imm8>]
    //   xxxh<c> <Rt>, [<Rn>, #+/-<Imm8>]!
    //
    // cccc000pu0wxnnnnttttiiiiyyyyjjjj where cccc=Cond, nnnn=Rn, tttt=Rt,
    // iiiijjjj=Imm8, pu0w<<21 is a BlockAddr, x000000000000yyyy0000=Opcode,
    // and pu0w0nnnn0000iiii0000jjjj=Address.
    assert(Rt < RegARM32::getNumGPRegs());
    assert(CondARM32::isDefined(Cond));
    verifyPOrNotW(Address, InstName);
    verifyRegNotPc(Rt, "Rt", InstName);
    if (isBitSet(W, Address))
      verifyRegsNotEq(getGPRReg(kRnShift, Address), "Rn", Rt, "Rt", InstName);
    const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) |
                             Opcode | (Rt << kRdShift) | Address;
    emitInst(Encoding);
    return;
  }
  case EncodedAsShiftRotateImm5: {
    // XXXH (register)
    //   xxxh<c> <Rt>, [<Rn>, +/-<Rm>]{!}
    //   xxxh<c> <Rt>, [<Rn>], +/-<Rm>
    //
    // cccc000pu0wxnnnntttt00001011mmmm where cccc=Cond, tttt=Rt, nnnn=Rn,
    // mmmm=Rm, pu0w<<21 is a BlockAddr, x000000000000yyyy0000=Opcode, and
    // pu0w0nnnn000000000000mmmm=Address.
    assert(Rt < RegARM32::getNumGPRegs());
    assert(CondARM32::isDefined(Cond));
    verifyPOrNotW(Address, InstName);
    verifyRegNotPc(Rt, "Rt", InstName);
    verifyAddrRegNotPc(kRmShift, Address, "Rm", InstName);
    const RegARM32::GPRRegister Rn = getGPRReg(kRnShift, Address);
    if (isBitSet(W, Address)) {
      verifyRegNotPc(Rn, "Rn", InstName);
      verifyRegsNotEq(Rn, "Rn", Rt, "Rt", InstName);
    }
    if (mask(Address, kShiftImmShift, 5) != 0)
      // For encoding 3, no shift is allowed.
      llvm::report_fatal_error(std::string(InstName) +
                               ": Shift constant not allowed");
    const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) |
                             Opcode | (Rt << kRdShift) | Address;
    emitInst(Encoding);
    return;
  }
  }
}

void AssemblerARM32::emitDivOp(CondARM32::Cond Cond, IValueT Opcode, IValueT Rd,
                               IValueT Rn, IValueT Rm) {
  assert(Rd < RegARM32::getNumGPRegs());
  assert(Rn < RegARM32::getNumGPRegs());
  assert(Rm < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  const IValueT Encoding = Opcode | (encodeCondition(Cond) << kConditionShift) |
                           (Rn << kDivRnShift) | (Rd << kDivRdShift) | B26 |
                           B25 | B24 | B20 | B15 | B14 | B13 | B12 | B4 |
                           (Rm << kDivRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::emitInsertExtractInt(CondARM32::Cond Cond,
                                          const Operand *OpQn, uint32_t Index,
                                          const Operand *OpRt, bool IsExtract,
                                          const char *InstName) {
  const IValueT Rt = encodeGPRegister(OpRt, "Rt", InstName);
  IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", InstName));
  assert(Rt != RegARM32::Encoded_Reg_pc);
  assert(Rt != RegARM32::Encoded_Reg_sp);
  assert(CondARM32::isDefined(Cond));
  const uint32_t BitSize = typeWidthInBytes(OpRt->getType()) * CHAR_BIT;
  IValueT Opcode1 = 0;
  IValueT Opcode2 = 0;
  switch (BitSize) {
  default:
    llvm::report_fatal_error(std::string(InstName) +
                             ": Unable to process type " +
                             typeStdString(OpRt->getType()));
  case 8:
    assert(Index < 16);
    Dn = Dn | mask(Index, 3, 1);
    Opcode1 = B1 | mask(Index, 2, 1);
    Opcode2 = mask(Index, 0, 2);
    break;
  case 16:
    assert(Index < 8);
    Dn = Dn | mask(Index, 2, 1);
    Opcode1 = mask(Index, 1, 1);
    Opcode2 = (mask(Index, 0, 1) << 1) | B0;
    break;
  case 32:
    assert(Index < 4);
    Dn = Dn | mask(Index, 1, 1);
    Opcode1 = mask(Index, 0, 1);
    break;
  }
  const IValueT Encoding = B27 | B26 | B25 | B11 | B9 | B8 | B4 |
                           (encodeCondition(Cond) << kConditionShift) |
                           (Opcode1 << 21) |
                           (getXXXXInRegYXXXX(Dn) << kRnShift) | (Rt << 12) |
                           (encodeBool(IsExtract) << 20) |
                           (getYInRegYXXXX(Dn) << 7) | (Opcode2 << 5);
  emitInst(Encoding);
}

void AssemblerARM32::emitMoveSS(CondARM32::Cond Cond, IValueT Sd, IValueT Sm) {
  // VMOV (register) - ARM section A8.8.340, encoding A2:
  //   vmov<c>.f32 <Sd>, <Sm>
  //
  // cccc11101D110000dddd101001M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr IValueT VmovssOpcode = B23 | B21 | B20 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VmovssOpcode, Sd, S0, Sm);
}

void AssemblerARM32::emitMulOp(CondARM32::Cond Cond, IValueT Opcode, IValueT Rd,
                               IValueT Rn, IValueT Rm, IValueT Rs,
                               bool SetFlags) {
  assert(Rd < RegARM32::getNumGPRegs());
  assert(Rn < RegARM32::getNumGPRegs());
  assert(Rm < RegARM32::getNumGPRegs());
  assert(Rs < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = Opcode | (encodeCondition(Cond) << kConditionShift) |
                     (encodeBool(SetFlags) << kSShift) | (Rn << kRnShift) |
                     (Rd << kRdShift) | (Rs << kRsShift) | B7 | B4 |
                     (Rm << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::emitMultiMemOp(CondARM32::Cond Cond,
                                    BlockAddressMode AddressMode, bool IsLoad,
                                    IValueT BaseReg, IValueT Registers) {
  assert(CondARM32::isDefined(Cond));
  assert(BaseReg < RegARM32::getNumGPRegs());
  assert(Registers < (1 << RegARM32::getNumGPRegs()));
  IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | B27 |
                     AddressMode | (IsLoad ? L : 0) | (BaseReg << kRnShift) |
                     Registers;
  emitInst(Encoding);
}

void AssemblerARM32::emitSignExtend(CondARM32::Cond Cond, IValueT Opcode,
                                    const Operand *OpRd, const Operand *OpSrc0,
                                    const char *InstName) {
  IValueT Rd = encodeGPRegister(OpRd, "Rd", InstName);
  IValueT Rm = encodeGPRegister(OpSrc0, "Rm", InstName);
  // Note: For the moment, we assume no rotation is specified.
  RotationValue Rotation = kRotateNone;
  constexpr IValueT Rn = RegARM32::Encoded_Reg_pc;
  const Type Ty = OpSrc0->getType();
  switch (Ty) {
  default:
    llvm::report_fatal_error(std::string(InstName) + ": Type " +
                             typeString(Ty) + " not allowed");
    break;
  case IceType_i1:
  case IceType_i8: {
    // SXTB/UXTB - Arm sections A8.8.233 and A8.8.274, encoding A1:
    //   sxtb<c> <Rd>, <Rm>{, <rotate>}
    //   uxtb<c> <Rd>, <Rm>{, <rotate>}
    //
    // ccccxxxxxxxx1111ddddrr000111mmmm where cccc=Cond, xxxxxxxx<<20=Opcode,
    // dddd=Rd, mmmm=Rm, and rr defined (RotationValue) rotate.
    break;
  }
  case IceType_i16: {
    // SXTH/UXTH - ARM sections A8.8.235 and A8.8.276, encoding A1:
    //   uxth<c> <Rd>< <Rm>{, <rotate>}
    //
    // cccc01101111nnnnddddrr000111mmmm where cccc=Cond, dddd=Rd, mmmm=Rm, and
    // rr defined (RotationValue) rotate.
    Opcode |= B20;
    break;
  }
  }

  assert(CondARM32::isDefined(Cond));
  IValueT Rot = encodeRotation(Rotation);
  if (!Utils::IsUint(2, Rot))
    llvm::report_fatal_error(std::string(InstName) +
                             ": Illegal rotation value");
  IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | Opcode |
                     (Rn << kRnShift) | (Rd << kRdShift) |
                     (Rot << kRotationShift) | B6 | B5 | B4 | (Rm << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::emitSIMDBase(IValueT Opcode, IValueT Dd, IValueT Dn,
                                  IValueT Dm, bool UseQRegs, bool IsFloatTy) {
  const IValueT Encoding =
      Opcode | B25 | (encodeCondition(CondARM32::kNone) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (getXXXXInRegYXXXX(Dn) << 16) |
      (getXXXXInRegYXXXX(Dd) << 12) | (IsFloatTy ? B10 : 0) |
      (getYInRegYXXXX(Dn) << 7) | (encodeBool(UseQRegs) << 6) |
      (getYInRegYXXXX(Dm) << 5) | getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::emitSIMD(IValueT Opcode, Type ElmtTy, IValueT Dd,
                              IValueT Dn, IValueT Dm, bool UseQRegs) {
  constexpr IValueT ElmtShift = 20;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));
  emitSIMDBase(Opcode | (ElmtSize << ElmtShift), Dd, Dn, Dm, UseQRegs,
               isFloatingType(ElmtTy));
}

void AssemblerARM32::emitSIMDqqqBase(IValueT Opcode, const Operand *OpQd,
                                     const Operand *OpQn, const Operand *OpQm,
                                     bool IsFloatTy, const char *OpcodeName) {
  const IValueT Qd = encodeQRegister(OpQd, "Qd", OpcodeName);
  const IValueT Qn = encodeQRegister(OpQn, "Qn", OpcodeName);
  const IValueT Qm = encodeQRegister(OpQm, "Qm", OpcodeName);
  constexpr bool UseQRegs = true;
  emitSIMDBase(Opcode, mapQRegToDReg(Qd), mapQRegToDReg(Qn), mapQRegToDReg(Qm),
               UseQRegs, IsFloatTy);
}

void AssemblerARM32::emitSIMDqqq(IValueT Opcode, Type ElmtTy,
                                 const Operand *OpQd, const Operand *OpQn,
                                 const Operand *OpQm, const char *OpcodeName) {
  constexpr IValueT ElmtShift = 20;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));
  emitSIMDqqqBase(Opcode | (ElmtSize << ElmtShift), OpQd, OpQn, OpQm,
                  isFloatingType(ElmtTy), OpcodeName);
}

void AssemblerARM32::emitSIMDShiftqqc(IValueT Opcode, const Operand *OpQd,
                                      const Operand *OpQm, const IValueT Imm6,
                                      const char *OpcodeName) {
  const IValueT Qd = encodeQRegister(OpQd, "Qd", OpcodeName);
  const IValueT Qn = 0;
  const IValueT Qm = encodeQRegister(OpQm, "Qm", OpcodeName);
  constexpr bool UseQRegs = true;
  constexpr bool IsFloatTy = false;
  constexpr IValueT ElmtShift = 16;
  emitSIMDBase(Opcode | (Imm6 << ElmtShift), mapQRegToDReg(Qd),
               mapQRegToDReg(Qn), mapQRegToDReg(Qm), UseQRegs, IsFloatTy);
}

void AssemblerARM32::emitSIMDCvtqq(IValueT Opcode, const Operand *OpQd,
                                   const Operand *OpQm,
                                   const char *OpcodeName) {
  const IValueT SIMDOpcode =
      B24 | B23 | B21 | B20 | B19 | B17 | B16 | B10 | B9 | Opcode;
  constexpr bool UseQRegs = true;
  constexpr bool IsFloatTy = false;
  const IValueT Qd = encodeQRegister(OpQd, "Qd", OpcodeName);
  constexpr IValueT Qn = 0;
  const IValueT Qm = encodeQRegister(OpQm, "Qm", OpcodeName);
  emitSIMDBase(SIMDOpcode, mapQRegToDReg(Qd), mapQRegToDReg(Qn),
               mapQRegToDReg(Qm), UseQRegs, IsFloatTy);
}

void AssemblerARM32::emitVFPddd(CondARM32::Cond Cond, IValueT Opcode,
                                IValueT Dd, IValueT Dn, IValueT Dm) {
  assert(Dd < RegARM32::getNumDRegs());
  assert(Dn < RegARM32::getNumDRegs());
  assert(Dm < RegARM32::getNumDRegs());
  assert(CondARM32::isDefined(Cond));
  constexpr IValueT VFPOpcode = B27 | B26 | B25 | B11 | B9 | B8;
  const IValueT Encoding =
      Opcode | VFPOpcode | (encodeCondition(Cond) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (getXXXXInRegYXXXX(Dn) << 16) |
      (getXXXXInRegYXXXX(Dd) << 12) | (getYInRegYXXXX(Dn) << 7) |
      (getYInRegYXXXX(Dm) << 5) | getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::emitVFPddd(CondARM32::Cond Cond, IValueT Opcode,
                                const Operand *OpDd, const Operand *OpDn,
                                const Operand *OpDm, const char *InstName) {
  IValueT Dd = encodeDRegister(OpDd, "Dd", InstName);
  IValueT Dn = encodeDRegister(OpDn, "Dn", InstName);
  IValueT Dm = encodeDRegister(OpDm, "Dm", InstName);
  emitVFPddd(Cond, Opcode, Dd, Dn, Dm);
}

void AssemblerARM32::emitVFPsss(CondARM32::Cond Cond, IValueT Opcode,
                                IValueT Sd, IValueT Sn, IValueT Sm) {
  assert(Sd < RegARM32::getNumSRegs());
  assert(Sn < RegARM32::getNumSRegs());
  assert(Sm < RegARM32::getNumSRegs());
  assert(CondARM32::isDefined(Cond));
  constexpr IValueT VFPOpcode = B27 | B26 | B25 | B11 | B9;
  const IValueT Encoding =
      Opcode | VFPOpcode | (encodeCondition(Cond) << kConditionShift) |
      (getYInRegXXXXY(Sd) << 22) | (getXXXXInRegXXXXY(Sn) << 16) |
      (getXXXXInRegXXXXY(Sd) << 12) | (getYInRegXXXXY(Sn) << 7) |
      (getYInRegXXXXY(Sm) << 5) | getXXXXInRegXXXXY(Sm);
  emitInst(Encoding);
}

void AssemblerARM32::emitVFPsss(CondARM32::Cond Cond, IValueT Opcode,
                                const Operand *OpSd, const Operand *OpSn,
                                const Operand *OpSm, const char *InstName) {
  const IValueT Sd = encodeSRegister(OpSd, "Sd", InstName);
  const IValueT Sn = encodeSRegister(OpSn, "Sn", InstName);
  const IValueT Sm = encodeSRegister(OpSm, "Sm", InstName);
  emitVFPsss(Cond, Opcode, Sd, Sn, Sm);
}

void AssemblerARM32::adc(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // ADC (register) - ARM section 18.8.2, encoding A1:
  //   adc{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  //
  // cccc0000101snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // ADC (Immediate) - ARM section A8.8.1, encoding A1:
  //   adc{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0010101snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *AdcName = "adc";
  constexpr IValueT AdcOpcode = B2 | B0; // 0101
  emitType01(Cond, AdcOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             AdcName);
}

void AssemblerARM32::add(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // ADD (register) - ARM section A8.8.7, encoding A1:
  //   add{s}<c> <Rd>, <Rn>, <Rm>{, <shiff>}
  // ADD (Sp plus register) - ARM section A8.8.11, encoding A1:
  //   add{s}<c> sp, <Rn>, <Rm>{, <shiff>}
  //
  // cccc0000100snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // ADD (Immediate) - ARM section A8.8.5, encoding A1:
  //   add{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  // ADD (SP plus immediate) - ARM section A8.8.9, encoding A1.
  //   add{s}<c> <Rd>, sp, #<RotatedImm8>
  //
  // cccc0010100snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *AddName = "add";
  constexpr IValueT Add = B2; // 0100
  emitType01(Cond, Add, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             AddName);
}

void AssemblerARM32::and_(const Operand *OpRd, const Operand *OpRn,
                          const Operand *OpSrc1, bool SetFlags,
                          CondARM32::Cond Cond) {
  // AND (register) - ARM section A8.8.14, encoding A1:
  //   and{s}<c> <Rd>, <Rn>{, <shift>}
  //
  // cccc0000000snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // AND (Immediate) - ARM section A8.8.13, encoding A1:
  //   and{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0010100snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *AndName = "and";
  constexpr IValueT And = 0; // 0000
  emitType01(Cond, And, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             AndName);
}

void AssemblerARM32::b(Label *L, CondARM32::Cond Cond) {
  emitBranch(L, Cond, false);
}

void AssemblerARM32::bkpt(uint16_t Imm16) {
  // BKPT - ARM section A*.8.24 - encoding A1:
  //   bkpt #<Imm16>
  //
  // cccc00010010iiiiiiiiiiii0111iiii where cccc=AL and iiiiiiiiiiiiiiii=Imm16
  const IValueT Encoding = (CondARM32::AL << kConditionShift) | B24 | B21 |
                           ((Imm16 >> 4) << 8) | B6 | B5 | B4 | (Imm16 & 0xf);
  emitInst(Encoding);
}

void AssemblerARM32::bic(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // BIC (register) - ARM section A8.8.22, encoding A1:
  //   bic{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  //
  // cccc0001110snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // BIC (immediate) - ARM section A8.8.21, encoding A1:
  //   bic{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0011110snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rn, nnnn=Rn,
  // s=SetFlags, and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *BicName = "bic";
  constexpr IValueT BicOpcode = B3 | B2 | B1; // i.e. 1110
  emitType01(Cond, BicOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             BicName);
}

void AssemblerARM32::bl(const ConstantRelocatable *Target) {
  // BL (immediate) - ARM section A8.8.25, encoding A1:
  //   bl<c> <label>
  //
  // cccc1011iiiiiiiiiiiiiiiiiiiiiiii where cccc=Cond (not currently allowed)
  // and iiiiiiiiiiiiiiiiiiiiiiii is the (encoded) Target to branch to.
  emitFixup(createBlFixup(Target));
  constexpr CondARM32::Cond Cond = CondARM32::AL;
  constexpr IValueT Immed = 0;
  constexpr bool Link = true;
  emitType05(Cond, Immed, Link);
}

void AssemblerARM32::blx(const Operand *Target) {
  // BLX (register) - ARM section A8.8.26, encoding A1:
  //   blx<c> <Rm>
  //
  // cccc000100101111111111110011mmmm where cccc=Cond (not currently allowed)
  // and mmmm=Rm.
  constexpr const char *BlxName = "Blx";
  IValueT Rm = encodeGPRegister(Target, "Rm", BlxName);
  verifyRegNotPc(Rm, "Rm", BlxName);
  constexpr CondARM32::Cond Cond = CondARM32::AL;
  int32_t Encoding = (encodeCondition(Cond) << kConditionShift) | B24 | B21 |
                     (0xfff << 8) | B5 | B4 | (Rm << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::bx(RegARM32::GPRRegister Rm, CondARM32::Cond Cond) {
  // BX - ARM section A8.8.27, encoding A1:
  //   bx<c> <Rm>
  //
  // cccc000100101111111111110001mmmm where mmmm=rm and cccc=Cond.
  assert(CondARM32::isDefined(Cond));
  const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | B24 |
                           B21 | (0xfff << 8) | B4 |
                           (encodeGPRRegister(Rm) << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::clz(const Operand *OpRd, const Operand *OpSrc,
                         CondARM32::Cond Cond) {
  // CLZ - ARM section A8.8.33, encoding A1:
  //   clz<c> <Rd> <Rm>
  //
  // cccc000101101111dddd11110001mmmm where cccc=Cond, dddd=Rd, and mmmm=Rm.
  constexpr const char *ClzName = "clz";
  constexpr const char *RdName = "Rd";
  constexpr const char *RmName = "Rm";
  IValueT Rd = encodeGPRegister(OpRd, RdName, ClzName);
  assert(Rd < RegARM32::getNumGPRegs());
  verifyRegNotPc(Rd, RdName, ClzName);
  IValueT Rm = encodeGPRegister(OpSrc, RmName, ClzName);
  assert(Rm < RegARM32::getNumGPRegs());
  verifyRegNotPc(Rm, RmName, ClzName);
  assert(CondARM32::isDefined(Cond));
  constexpr IValueT PredefinedBits =
      B24 | B22 | B21 | (0xF << 16) | (0xf << 8) | B4;
  const IValueT Encoding = PredefinedBits | (Cond << kConditionShift) |
                           (Rd << kRdShift) | (Rm << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::cmn(const Operand *OpRn, const Operand *OpSrc1,
                         CondARM32::Cond Cond) {
  // CMN (immediate) - ARM section A8.8.34, encoding A1:
  //   cmn<c> <Rn>, #<RotatedImm8>
  //
  // cccc00110111nnnn0000iiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  //
  // CMN (register) - ARM section A8.8.35, encodeing A1:
  //   cmn<c> <Rn>, <Rm>{, <shift>}
  //
  // cccc00010111nnnn0000iiiiitt0mmmm where cccc=Cond, nnnn=Rn, mmmm=Rm,
  // iiiii=Shift, and tt=ShiftKind.
  constexpr const char *CmnName = "cmn";
  constexpr IValueT CmnOpcode = B3 | B1 | B0; // ie. 1011
  emitCompareOp(Cond, CmnOpcode, OpRn, OpSrc1, CmnName);
}

void AssemblerARM32::cmp(const Operand *OpRn, const Operand *OpSrc1,
                         CondARM32::Cond Cond) {
  // CMP (register) - ARM section A8.8.38, encoding A1:
  //   cmp<c> <Rn>, <Rm>{, <shift>}
  //
  // cccc00010101nnnn0000iiiiitt0mmmm where cccc=Cond, nnnn=Rn, mmmm=Rm,
  // iiiii=Shift, and tt=ShiftKind.
  //
  // CMP (immediate) - ARM section A8.8.37
  //  cmp<c: <Rn>, #<RotatedImm8>
  //
  // cccc00110101nnnn0000iiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *CmpName = "cmp";
  constexpr IValueT CmpOpcode = B3 | B1; // ie. 1010
  emitCompareOp(Cond, CmpOpcode, OpRn, OpSrc1, CmpName);
}

void AssemblerARM32::dmb(IValueT Option) {
  // DMB - ARM section A8.8.43, encoding A1:
  //   dmb <option>
  //
  // 1111010101111111111100000101xxxx where xxxx=Option.
  assert(Utils::IsUint(4, Option) && "Bad dmb option");
  const IValueT Encoding =
      (encodeCondition(CondARM32::kNone) << kConditionShift) | B26 | B24 | B22 |
      B21 | B20 | B19 | B18 | B17 | B16 | B15 | B14 | B13 | B12 | B6 | B4 |
      Option;
  emitInst(Encoding);
}

void AssemblerARM32::eor(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // EOR (register) - ARM section A*.8.47, encoding A1:
  //   eor{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  //
  // cccc0000001snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // EOR (Immediate) - ARM section A8.*.46, encoding A1:
  //   eor{s}<c> <Rd>, <Rn>, #RotatedImm8
  //
  // cccc0010001snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *EorName = "eor";
  constexpr IValueT EorOpcode = B0; // 0001
  emitType01(Cond, EorOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             EorName);
}

void AssemblerARM32::ldr(const Operand *OpRt, const Operand *OpAddress,
                         CondARM32::Cond Cond, const TargetInfo &TInfo) {
  constexpr const char *LdrName = "ldr";
  constexpr bool IsLoad = true;
  IValueT Rt = encodeGPRegister(OpRt, "Rt", LdrName);
  const Type Ty = OpRt->getType();
  switch (Ty) {
  case IceType_i64:
    // LDRD is not implemented because target lowering handles i64 and double by
    // using two (32-bit) load instructions. Note: Intentionally drop to default
    // case.
    llvm::report_fatal_error(std::string("ldr : Type ") + typeString(Ty) +
                             " not implemented");
  default:
    llvm::report_fatal_error(std::string("ldr : Type ") + typeString(Ty) +
                             " not allowed");
  case IceType_i1:
  case IceType_i8: {
    // LDRB (immediate) - ARM section A8.8.68, encoding A1:
    //   ldrb<c> <Rt>, [<Rn>{, #+/-<imm12>}]     ; p=1, w=0
    //   ldrb<c> <Rt>, [<Rn>], #+/-<imm12>       ; p=1, w=1
    //   ldrb<c> <Rt>, [<Rn>, #+/-<imm12>]!      ; p=0, w=1
    //
    // cccc010pu1w1nnnnttttiiiiiiiiiiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiiiiiii=imm12, u=1 if +, pu0w is a BlockAddr, and
    // pu0w0nnnn0000iiiiiiiiiiii=Address.
    //
    // LDRB (register) - ARM section A8.8.66, encoding A1:
    //   ldrb<c> <Rt>, [<Rn>, +/-<Rm>{, <shift>}]{!}
    //   ldrb<c> <Rt>, [<Rn>], +/-<Rm>{, <shift>}
    //
    // cccc011pu1w1nnnnttttiiiiiss0mmmm where cccc=Cond, tttt=Rt, U=1 if +, pu0b
    // is a BlockAddr, and pu0w0nnnn0000iiiiiss0mmmm=Address.
    constexpr bool IsByte = true;
    emitMemOp(Cond, IsLoad, IsByte, Rt, OpAddress, TInfo, LdrName);
    return;
  }
  case IceType_i16: {
    // LDRH (immediate) - ARM section A8.8.80, encoding A1:
    //   ldrh<c> <Rt>, [<Rn>{, #+/-<Imm8>}]
    //   ldrh<c> <Rt>, [<Rn>], #+/-<Imm8>
    //   ldrh<c> <Rt>, [<Rn>, #+/-<Imm8>]!
    //
    // cccc000pu1w1nnnnttttiiii1011iiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiii=Imm8, u=1 if +, pu0w is a BlockAddr, and
    // pu0w0nnnn0000iiiiiiiiiiii=Address.
    constexpr const char *Ldrh = "ldrh";
    emitMemOpEnc3(Cond, L | B7 | B5 | B4, Rt, OpAddress, TInfo, Ldrh);
    return;
  }
  case IceType_i32: {
    // LDR (immediate) - ARM section A8.8.63, encoding A1:
    //   ldr<c> <Rt>, [<Rn>{, #+/-<imm12>}]      ; p=1, w=0
    //   ldr<c> <Rt>, [<Rn>], #+/-<imm12>        ; p=1, w=1
    //   ldr<c> <Rt>, [<Rn>, #+/-<imm12>]!       ; p=0, w=1
    //
    // cccc010pu0w1nnnnttttiiiiiiiiiiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiiiiiii=imm12, u=1 if +, pu0w is a BlockAddr, and
    //
    // LDR (register) - ARM section A8.8.70, encoding A1:
    //   ldrb<c> <Rt>, [<Rn>, +/-<Rm>{, <shift>}]{!}
    //   ldrb<c> <Rt>, [<Rn>], +-<Rm>{, <shift>}
    //
    // cccc011pu0w1nnnnttttiiiiiss0mmmm where cccc=Cond, tttt=Rt, U=1 if +, pu0b
    // is a BlockAddr, and pu0w0nnnn0000iiiiiss0mmmm=Address.
    constexpr bool IsByte = false;
    emitMemOp(Cond, IsLoad, IsByte, Rt, OpAddress, TInfo, LdrName);
    return;
  }
  }
}

void AssemblerARM32::emitMemExOp(CondARM32::Cond Cond, Type Ty, bool IsLoad,
                                 const Operand *OpRd, IValueT Rt,
                                 const Operand *OpAddress,
                                 const TargetInfo &TInfo,
                                 const char *InstName) {
  IValueT Rd = encodeGPRegister(OpRd, "Rd", InstName);
  IValueT MemExOpcode = IsLoad ? B0 : 0;
  switch (Ty) {
  default:
    llvm::report_fatal_error(std::string(InstName) + ": Type " +
                             typeString(Ty) + " not allowed");
  case IceType_i1:
  case IceType_i8:
    MemExOpcode |= B2;
    break;
  case IceType_i16:
    MemExOpcode |= B2 | B1;
    break;
  case IceType_i32:
    break;
  case IceType_i64:
    MemExOpcode |= B1;
  }
  IValueT AddressRn;
  if (encodeAddress(OpAddress, AddressRn, TInfo, NoImmOffsetAddress) !=
      EncodedAsImmRegOffset)
    llvm::report_fatal_error(std::string(InstName) +
                             ": Can't extract Rn from address");
  assert(Utils::IsAbsoluteUint(3, MemExOpcode));
  assert(Rd < RegARM32::getNumGPRegs());
  assert(Rt < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = (Cond << kConditionShift) | B24 | B23 | B11 | B10 | B9 |
                     B8 | B7 | B4 | (MemExOpcode << kMemExOpcodeShift) |
                     AddressRn | (Rd << kRdShift) | (Rt << kRmShift);
  emitInst(Encoding);
  return;
}

void AssemblerARM32::ldrex(const Operand *OpRt, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // LDREXB - ARM section A8.8.76, encoding A1:
  //   ldrexb<c> <Rt>, [<Rn>]
  //
  // cccc00011101nnnntttt111110011111 where cccc=Cond, tttt=Rt, and nnnn=Rn.
  //
  // LDREXH - ARM section A8.8.78, encoding A1:
  //   ldrexh<c> <Rt>, [<Rn>]
  //
  // cccc00011111nnnntttt111110011111 where cccc=Cond, tttt=Rt, and nnnn=Rn.
  //
  // LDREX - ARM section A8.8.75, encoding A1:
  //   ldrex<c> <Rt>, [<Rn>]
  //
  // cccc00011001nnnntttt111110011111 where cccc=Cond, tttt=Rt, and nnnn=Rn.
  //
  // LDREXD - ARM section A8.
  //   ldrexd<c> <Rt>, [<Rn>]
  //
  // cccc00011001nnnntttt111110011111 where cccc=Cond, tttt=Rt, and nnnn=Rn.
  constexpr const char *LdrexName = "ldrex";
  const Type Ty = OpRt->getType();
  constexpr bool IsLoad = true;
  constexpr IValueT Rm = RegARM32::Encoded_Reg_pc;
  emitMemExOp(Cond, Ty, IsLoad, OpRt, Rm, OpAddress, TInfo, LdrexName);
}

void AssemblerARM32::emitShift(const CondARM32::Cond Cond,
                               const OperandARM32::ShiftKind Shift,
                               const Operand *OpRd, const Operand *OpRm,
                               const Operand *OpSrc1, const bool SetFlags,
                               const char *InstName) {
  constexpr IValueT ShiftOpcode = B3 | B2 | B0; // 1101
  IValueT Rd = encodeGPRegister(OpRd, "Rd", InstName);
  IValueT Rm = encodeGPRegister(OpRm, "Rm", InstName);
  IValueT Value;
  switch (encodeOperand(OpSrc1, Value, WantGPRegs)) {
  default:
    llvm::report_fatal_error(std::string(InstName) +
                             ": Last operand not understood");
  case EncodedAsShiftImm5: {
    // XXX (immediate)
    //   xxx{s}<c> <Rd>, <Rm>, #imm5
    //
    // cccc0001101s0000ddddiiiii000mmmm where cccc=Cond, s=SetFlags, dddd=Rd,
    // iiiii=imm5, and mmmm=Rm.
    constexpr IValueT Rn = 0; // Rn field is not used.
    Value = Value | (Rm << kRmShift) | (Shift << kShiftShift);
    emitType01(Cond, kInstTypeDataRegShift, ShiftOpcode, SetFlags, Rn, Rd,
               Value, RdIsPcAndSetFlags, InstName);
    return;
  }
  case EncodedAsRegister: {
    // XXX (register)
    //   xxx{S}<c> <Rd>, <Rm>, <Rs>
    //
    // cccc0001101s0000ddddssss0001mmmm where cccc=Cond, s=SetFlags, dddd=Rd,
    // mmmm=Rm, and ssss=Rs.
    constexpr IValueT Rn = 0; // Rn field is not used.
    IValueT Rs = encodeGPRegister(OpSrc1, "Rs", InstName);
    verifyRegNotPc(Rd, "Rd", InstName);
    verifyRegNotPc(Rm, "Rm", InstName);
    verifyRegNotPc(Rs, "Rs", InstName);
    emitType01(Cond, kInstTypeDataRegShift, ShiftOpcode, SetFlags, Rn, Rd,
               encodeShiftRotateReg(Rm, Shift, Rs), NoChecks, InstName);
    return;
  }
  }
}

void AssemblerARM32::asr(const Operand *OpRd, const Operand *OpRm,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  constexpr const char *AsrName = "asr";
  emitShift(Cond, OperandARM32::ASR, OpRd, OpRm, OpSrc1, SetFlags, AsrName);
}

void AssemblerARM32::lsl(const Operand *OpRd, const Operand *OpRm,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  constexpr const char *LslName = "lsl";
  emitShift(Cond, OperandARM32::LSL, OpRd, OpRm, OpSrc1, SetFlags, LslName);
}

void AssemblerARM32::lsr(const Operand *OpRd, const Operand *OpRm,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  constexpr const char *LsrName = "lsr";
  emitShift(Cond, OperandARM32::LSR, OpRd, OpRm, OpSrc1, SetFlags, LsrName);
}

void AssemblerARM32::mov(const Operand *OpRd, const Operand *OpSrc,
                         CondARM32::Cond Cond) {
  // MOV (register) - ARM section A8.8.104, encoding A1:
  //   mov{S}<c> <Rd>, <Rn>
  //
  // cccc0001101s0000dddd00000000mmmm where cccc=Cond, s=SetFlags, dddd=Rd,
  // and nnnn=Rn.
  //
  // MOV (immediate) - ARM section A8.8.102, encoding A1:
  //   mov{S}<c> <Rd>, #<RotatedImm8>
  //
  // cccc0011101s0000ddddiiiiiiiiiiii where cccc=Cond, s=SetFlags, dddd=Rd,
  // and iiiiiiiiiiii=RotatedImm8=Src.  Note: We don't use movs in this
  // assembler.
  constexpr const char *MovName = "mov";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MovName);
  constexpr bool SetFlags = false;
  constexpr IValueT Rn = 0;
  constexpr IValueT MovOpcode = B3 | B2 | B0; // 1101.
  emitType01(Cond, MovOpcode, Rd, Rn, OpSrc, SetFlags, RdIsPcAndSetFlags,
             MovName);
}

void AssemblerARM32::emitMovwt(CondARM32::Cond Cond, bool IsMovW,
                               const Operand *OpRd, const Operand *OpSrc,
                               const char *MovName) {
  IValueT Opcode = B25 | B24 | (IsMovW ? 0 : B22);
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MovName);
  IValueT Imm16;
  if (const auto *Src = llvm::dyn_cast<ConstantRelocatable>(OpSrc)) {
    emitFixup(createMoveFixup(IsMovW, Src));
    // Use 0 for the lower 16 bits of the relocatable, and add a fixup to
    // install the correct bits.
    Imm16 = 0;
  } else if (encodeOperand(OpSrc, Imm16, WantGPRegs) != EncodedAsConstI32) {
    llvm::report_fatal_error(std::string(MovName) + ": Not i32 constant");
  }
  assert(CondARM32::isDefined(Cond));
  if (!Utils::IsAbsoluteUint(16, Imm16))
    llvm::report_fatal_error(std::string(MovName) + ": Constant not i16");
  const IValueT Encoding = encodeCondition(Cond) << kConditionShift | Opcode |
                           ((Imm16 >> 12) << 16) | Rd << kRdShift |
                           (Imm16 & 0xfff);
  emitInst(Encoding);
}

void AssemblerARM32::movw(const Operand *OpRd, const Operand *OpSrc,
                          CondARM32::Cond Cond) {
  // MOV (immediate) - ARM section A8.8.102, encoding A2:
  //  movw<c> <Rd>, #<imm16>
  //
  // cccc00110000iiiiddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, and
  // iiiiiiiiiiiiiiii=imm16.
  constexpr const char *MovwName = "movw";
  constexpr bool IsMovW = true;
  emitMovwt(Cond, IsMovW, OpRd, OpSrc, MovwName);
}

void AssemblerARM32::movt(const Operand *OpRd, const Operand *OpSrc,
                          CondARM32::Cond Cond) {
  // MOVT - ARM section A8.8.106, encoding A1:
  //  movt<c> <Rd>, #<imm16>
  //
  // cccc00110100iiiiddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, and
  // iiiiiiiiiiiiiiii=imm16.
  constexpr const char *MovtName = "movt";
  constexpr bool IsMovW = false;
  emitMovwt(Cond, IsMovW, OpRd, OpSrc, MovtName);
}

void AssemblerARM32::mvn(const Operand *OpRd, const Operand *OpSrc,
                         CondARM32::Cond Cond) {
  // MVN (immediate) - ARM section A8.8.115, encoding A1:
  //   mvn{s}<c> <Rd>, #<const>
  //
  // cccc0011111s0000ddddiiiiiiiiiiii where cccc=Cond, s=SetFlags=0, dddd=Rd,
  // and iiiiiiiiiiii=const
  //
  // MVN (register) - ARM section A8.8.116, encoding A1:
  //   mvn{s}<c> <Rd>, <Rm>{, <shift>
  //
  // cccc0001111s0000ddddiiiiitt0mmmm where cccc=Cond, s=SetFlags=0, dddd=Rd,
  // mmmm=Rm, iiii defines shift constant, and tt=ShiftKind.
  constexpr const char *MvnName = "mvn";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MvnName);
  constexpr IValueT MvnOpcode = B3 | B2 | B1 | B0; // i.e. 1111
  constexpr IValueT Rn = 0;
  constexpr bool SetFlags = false;
  emitType01(Cond, MvnOpcode, Rd, Rn, OpSrc, SetFlags, RdIsPcAndSetFlags,
             MvnName);
}

void AssemblerARM32::nop() {
  // NOP - Section A8.8.119, encoding A1:
  //  nop<c>
  //
  // cccc0011001000001111000000000000 where cccc=Cond.
  constexpr CondARM32::Cond Cond = CondARM32::AL;
  const IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | B25 |
                           B24 | B21 | B15 | B14 | B13 | B12;
  emitInst(Encoding);
}

void AssemblerARM32::sbc(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // SBC (register) - ARM section 18.8.162, encoding A1:
  //   sbc{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  //
  // cccc0000110snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=Shift, tt=ShiftKind, and s=SetFlags.
  //
  // SBC (Immediate) - ARM section A8.8.161, encoding A1:
  //   sbc{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0010110snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *SbcName = "sbc";
  constexpr IValueT SbcOpcode = B2 | B1; // 0110
  emitType01(Cond, SbcOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             SbcName);
}

void AssemblerARM32::sdiv(const Operand *OpRd, const Operand *OpRn,
                          const Operand *OpSrc1, CondARM32::Cond Cond) {
  // SDIV - ARM section A8.8.165, encoding A1.
  //   sdiv<c> <Rd>, <Rn>, <Rm>
  //
  // cccc01110001dddd1111mmmm0001nnnn where cccc=Cond, dddd=Rd, nnnn=Rn, and
  // mmmm=Rm.
  constexpr const char *SdivName = "sdiv";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", SdivName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", SdivName);
  IValueT Rm = encodeGPRegister(OpSrc1, "Rm", SdivName);
  verifyRegNotPc(Rd, "Rd", SdivName);
  verifyRegNotPc(Rn, "Rn", SdivName);
  verifyRegNotPc(Rm, "Rm", SdivName);
  // Assembler registers rd, rn, rm are encoded as rn, rm, rs.
  constexpr IValueT SdivOpcode = 0;
  emitDivOp(Cond, SdivOpcode, Rd, Rn, Rm);
}

void AssemblerARM32::str(const Operand *OpRt, const Operand *OpAddress,
                         CondARM32::Cond Cond, const TargetInfo &TInfo) {
  constexpr const char *StrName = "str";
  constexpr bool IsLoad = false;
  IValueT Rt = encodeGPRegister(OpRt, "Rt", StrName);
  const Type Ty = OpRt->getType();
  switch (Ty) {
  case IceType_i64:
    // STRD is not implemented because target lowering handles i64 and double by
    // using two (32-bit) store instructions.  Note: Intentionally drop to
    // default case.
    llvm::report_fatal_error(std::string(StrName) + ": Type " + typeString(Ty) +
                             " not implemented");
  default:
    llvm::report_fatal_error(std::string(StrName) + ": Type " + typeString(Ty) +
                             " not allowed");
  case IceType_i1:
  case IceType_i8: {
    // STRB (immediate) - ARM section A8.8.207, encoding A1:
    //   strb<c> <Rt>, [<Rn>{, #+/-<imm12>}]     ; p=1, w=0
    //   strb<c> <Rt>, [<Rn>], #+/-<imm12>       ; p=1, w=1
    //   strb<c> <Rt>, [<Rn>, #+/-<imm12>]!      ; p=0, w=1
    //
    // cccc010pu1w0nnnnttttiiiiiiiiiiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiiiiiii=imm12, u=1 if +.
    constexpr bool IsByte = true;
    emitMemOp(Cond, IsLoad, IsByte, Rt, OpAddress, TInfo, StrName);
    return;
  }
  case IceType_i16: {
    // STRH (immediate) - ARM section A8.*.217, encoding A1:
    //   strh<c> <Rt>, [<Rn>{, #+/-<Imm8>}]
    //   strh<c> <Rt>, [<Rn>], #+/-<Imm8>
    //   strh<c> <Rt>, [<Rn>, #+/-<Imm8>]!
    //
    // cccc000pu1w0nnnnttttiiii1011iiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiii=Imm8, u=1 if +, pu0w is a BlockAddr, and
    // pu0w0nnnn0000iiiiiiiiiiii=Address.
    constexpr const char *Strh = "strh";
    emitMemOpEnc3(Cond, B7 | B5 | B4, Rt, OpAddress, TInfo, Strh);
    return;
  }
  case IceType_i32: {
    // Note: Handles i32 and float stores. Target lowering handles i64 and
    // double by using two (32 bit) store instructions.
    //
    // STR (immediate) - ARM section A8.8.207, encoding A1:
    //   str<c> <Rt>, [<Rn>{, #+/-<imm12>}]     ; p=1, w=0
    //   str<c> <Rt>, [<Rn>], #+/-<imm12>       ; p=1, w=1
    //   str<c> <Rt>, [<Rn>, #+/-<imm12>]!      ; p=0, w=1
    //
    // cccc010pu1w0nnnnttttiiiiiiiiiiii where cccc=Cond, tttt=Rt, nnnn=Rn,
    // iiiiiiiiiiii=imm12, u=1 if +.
    constexpr bool IsByte = false;
    emitMemOp(Cond, IsLoad, IsByte, Rt, OpAddress, TInfo, StrName);
    return;
  }
  }
}

void AssemblerARM32::strex(const Operand *OpRd, const Operand *OpRt,
                           const Operand *OpAddress, CondARM32::Cond Cond,
                           const TargetInfo &TInfo) {
  // STREXB - ARM section A8.8.213, encoding A1:
  //   strexb<c> <Rd>, <Rt>, [<Rn>]
  //
  // cccc00011100nnnndddd11111001tttt where cccc=Cond, dddd=Rd, tttt=Rt, and
  // nnnn=Rn.
  //
  // STREXH - ARM section A8.8.215, encoding A1:
  //   strexh<c> <Rd>, <Rt>, [<Rn>]
  //
  // cccc00011110nnnndddd11111001tttt where cccc=Cond, dddd=Rd, tttt=Rt, and
  // nnnn=Rn.
  //
  // STREX - ARM section A8.8.212, encoding A1:
  //   strex<c> <Rd>, <Rt>, [<Rn>]
  //
  // cccc00011000nnnndddd11111001tttt where cccc=Cond, dddd=Rd, tttt=Rt, and
  // nnnn=Rn.
  //
  // STREXD - ARM section A8.8.214, encoding A1:
  //   strexd<c> <Rd>, <Rt>, [<Rn>]
  //
  // cccc00011010nnnndddd11111001tttt where cccc=Cond, dddd=Rd, tttt=Rt, and
  // nnnn=Rn.
  constexpr const char *StrexName = "strex";
  // Note: Rt uses Rm shift in encoding.
  IValueT Rt = encodeGPRegister(OpRt, "Rt", StrexName);
  const Type Ty = OpRt->getType();
  constexpr bool IsLoad = true;
  emitMemExOp(Cond, Ty, !IsLoad, OpRd, Rt, OpAddress, TInfo, StrexName);
}

void AssemblerARM32::orr(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // ORR (register) - ARM Section A8.8.123, encoding A1:
  //   orr{s}<c> <Rd>, <Rn>, <Rm>
  //
  // cccc0001100snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=shift, tt=ShiftKind,, and s=SetFlags.
  //
  // ORR (register) - ARM Section A8.8.123, encoding A1:
  //   orr{s}<c> <Rd>, <Rn>,  #<RotatedImm8>
  //
  // cccc0001100snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8.
  constexpr const char *OrrName = "orr";
  constexpr IValueT OrrOpcode = B3 | B2; // i.e. 1100
  emitType01(Cond, OrrOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             OrrName);
}

void AssemblerARM32::pop(const Variable *OpRt, CondARM32::Cond Cond) {
  // POP - ARM section A8.8.132, encoding A2:
  //   pop<c> {Rt}
  //
  // cccc010010011101dddd000000000100 where dddd=Rt and cccc=Cond.
  constexpr const char *Pop = "pop";
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Pop);
  verifyRegsNotEq(Rt, "Rt", RegARM32::Encoded_Reg_sp, "sp", Pop);
  // Same as load instruction.
  constexpr bool IsLoad = true;
  constexpr bool IsByte = false;
  constexpr IOffsetT MaxOffset = (1 << 8) - 1;
  constexpr IValueT NoShiftRight = 0;
  IValueT Address =
      encodeImmRegOffset(RegARM32::Encoded_Reg_sp, kWordSize,
                         OperandARM32Mem::PostIndex, MaxOffset, NoShiftRight);
  emitMemOp(Cond, kInstTypeMemImmediate, IsLoad, IsByte, Rt, Address);
}

void AssemblerARM32::popList(const IValueT Registers, CondARM32::Cond Cond) {
  // POP - ARM section A8.*.131, encoding A1:
  //   pop<c> <registers>
  //
  // cccc100010111101rrrrrrrrrrrrrrrr where cccc=Cond and
  // rrrrrrrrrrrrrrrr=Registers (one bit for each GP register).
  constexpr bool IsLoad = true;
  emitMultiMemOp(Cond, IA_W, IsLoad, RegARM32::Encoded_Reg_sp, Registers);
}

void AssemblerARM32::push(const Operand *OpRt, CondARM32::Cond Cond) {
  // PUSH - ARM section A8.8.133, encoding A2:
  //   push<c> {Rt}
  //
  // cccc010100101101dddd000000000100 where dddd=Rt and cccc=Cond.
  constexpr const char *Push = "push";
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Push);
  verifyRegsNotEq(Rt, "Rt", RegARM32::Encoded_Reg_sp, "sp", Push);
  // Same as store instruction.
  constexpr bool isLoad = false;
  constexpr bool isByte = false;
  constexpr IOffsetT MaxOffset = (1 << 8) - 1;
  constexpr IValueT NoShiftRight = 0;
  IValueT Address =
      encodeImmRegOffset(RegARM32::Encoded_Reg_sp, -kWordSize,
                         OperandARM32Mem::PreIndex, MaxOffset, NoShiftRight);
  emitMemOp(Cond, kInstTypeMemImmediate, isLoad, isByte, Rt, Address);
}

void AssemblerARM32::pushList(const IValueT Registers, CondARM32::Cond Cond) {
  // PUSH - ARM section A8.8.133, encoding A1:
  //   push<c> <Registers>
  //
  // cccc100100101101rrrrrrrrrrrrrrrr where cccc=Cond and
  // rrrrrrrrrrrrrrrr=Registers (one bit for each GP register).
  constexpr bool IsLoad = false;
  emitMultiMemOp(Cond, DB_W, IsLoad, RegARM32::Encoded_Reg_sp, Registers);
}

void AssemblerARM32::mla(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpRm, const Operand *OpRa,
                         CondARM32::Cond Cond) {
  // MLA - ARM section A8.8.114, encoding A1.
  //   mla{s}<c> <Rd>, <Rn>, <Rm>, <Ra>
  //
  // cccc0000001sddddaaaammmm1001nnnn where cccc=Cond, s=SetFlags, dddd=Rd,
  // aaaa=Ra, mmmm=Rm, and nnnn=Rn.
  constexpr const char *MlaName = "mla";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MlaName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", MlaName);
  IValueT Rm = encodeGPRegister(OpRm, "Rm", MlaName);
  IValueT Ra = encodeGPRegister(OpRa, "Ra", MlaName);
  verifyRegNotPc(Rd, "Rd", MlaName);
  verifyRegNotPc(Rn, "Rn", MlaName);
  verifyRegNotPc(Rm, "Rm", MlaName);
  verifyRegNotPc(Ra, "Ra", MlaName);
  constexpr IValueT MlaOpcode = B21;
  constexpr bool SetFlags = true;
  // Assembler registers rd, rn, rm, ra are encoded as rn, rm, rs, rd.
  emitMulOp(Cond, MlaOpcode, Ra, Rd, Rn, Rm, !SetFlags);
}

void AssemblerARM32::mls(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpRm, const Operand *OpRa,
                         CondARM32::Cond Cond) {
  constexpr const char *MlsName = "mls";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MlsName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", MlsName);
  IValueT Rm = encodeGPRegister(OpRm, "Rm", MlsName);
  IValueT Ra = encodeGPRegister(OpRa, "Ra", MlsName);
  verifyRegNotPc(Rd, "Rd", MlsName);
  verifyRegNotPc(Rn, "Rn", MlsName);
  verifyRegNotPc(Rm, "Rm", MlsName);
  verifyRegNotPc(Ra, "Ra", MlsName);
  constexpr IValueT MlsOpcode = B22 | B21;
  constexpr bool SetFlags = true;
  // Assembler registers rd, rn, rm, ra are encoded as rn, rm, rs, rd.
  emitMulOp(Cond, MlsOpcode, Ra, Rd, Rn, Rm, !SetFlags);
}

void AssemblerARM32::mul(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // MUL - ARM section A8.8.114, encoding A1.
  //   mul{s}<c> <Rd>, <Rn>, <Rm>
  //
  // cccc0000000sdddd0000mmmm1001nnnn where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, and s=SetFlags.
  constexpr const char *MulName = "mul";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", MulName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", MulName);
  IValueT Rm = encodeGPRegister(OpSrc1, "Rm", MulName);
  verifyRegNotPc(Rd, "Rd", MulName);
  verifyRegNotPc(Rn, "Rn", MulName);
  verifyRegNotPc(Rm, "Rm", MulName);
  // Assembler registers rd, rn, rm are encoded as rn, rm, rs.
  constexpr IValueT MulOpcode = 0;
  emitMulOp(Cond, MulOpcode, RegARM32::Encoded_Reg_r0, Rd, Rn, Rm, SetFlags);
}

void AssemblerARM32::emitRdRm(CondARM32::Cond Cond, IValueT Opcode,
                              const Operand *OpRd, const Operand *OpRm,
                              const char *InstName) {
  IValueT Rd = encodeGPRegister(OpRd, "Rd", InstName);
  IValueT Rm = encodeGPRegister(OpRm, "Rm", InstName);
  IValueT Encoding =
      (Cond << kConditionShift) | Opcode | (Rd << kRdShift) | (Rm << kRmShift);
  emitInst(Encoding);
}

void AssemblerARM32::rbit(const Operand *OpRd, const Operand *OpRm,
                          CondARM32::Cond Cond) {
  // RBIT - ARM section A8.8.144, encoding A1:
  //   rbit<c> <Rd>, <Rm>
  //
  // cccc011011111111dddd11110011mmmm where cccc=Cond, dddd=Rn, and mmmm=Rm.
  constexpr const char *RbitName = "rev";
  constexpr IValueT RbitOpcode = B26 | B25 | B23 | B22 | B21 | B20 | B19 | B18 |
                                 B17 | B16 | B11 | B10 | B9 | B8 | B5 | B4;
  emitRdRm(Cond, RbitOpcode, OpRd, OpRm, RbitName);
}

void AssemblerARM32::rev(const Operand *OpRd, const Operand *OpRm,
                         CondARM32::Cond Cond) {
  // REV - ARM section A8.8.145, encoding A1:
  //   rev<c> <Rd>, <Rm>
  //
  // cccc011010111111dddd11110011mmmm where cccc=Cond, dddd=Rn, and mmmm=Rm.
  constexpr const char *RevName = "rev";
  constexpr IValueT RevOpcode = B26 | B25 | B23 | B21 | B20 | B19 | B18 | B17 |
                                B16 | B11 | B10 | B9 | B8 | B5 | B4;
  emitRdRm(Cond, RevOpcode, OpRd, OpRm, RevName);
}

void AssemblerARM32::rsb(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // RSB (immediate) - ARM section A8.8.152, encoding A1.
  //   rsb{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0010011snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=setFlags and iiiiiiiiiiii defines the RotatedImm8 value.
  //
  // RSB (register) - ARM section A8.8.163, encoding A1.
  //   rsb{s}<c> <Rd>, <Rn>, <Rm>{, <Shift>}
  //
  // cccc0000011snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=shift, tt==ShiftKind, and s=SetFlags.
  constexpr const char *RsbName = "rsb";
  constexpr IValueT RsbOpcode = B1 | B0; // 0011
  emitType01(Cond, RsbOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             RsbName);
}

void AssemblerARM32::rsc(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // RSC (immediate) - ARM section A8.8.155, encoding A1:
  //   rsc{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  //
  // cccc0010111snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=shift, tt=ShiftKind, and s=SetFlags.
  //
  // RSC (register) - ARM section A8.8.156, encoding A1:
  //   rsc{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  //
  // cccc0000111snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=shift, tt=ShiftKind, and s=SetFlags.
  //
  // RSC (register-shifted register) - ARM section A8.8.157, encoding A1:
  //   rsc{s}<c> <Rd>, <Rn>, <Rm>, <type> <Rs>
  //
  // cccc0000111fnnnnddddssss0tt1mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, ssss=Rs, tt defined <type>, and f=SetFlags.
  constexpr const char *RscName = "rsc";
  constexpr IValueT RscOpcode = B2 | B1 | B0; // i.e. 0111.
  emitType01(Cond, RscOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             RscName);
}

void AssemblerARM32::sxt(const Operand *OpRd, const Operand *OpSrc0,
                         CondARM32::Cond Cond) {
  constexpr const char *SxtName = "sxt";
  constexpr IValueT SxtOpcode = B26 | B25 | B23 | B21;
  emitSignExtend(Cond, SxtOpcode, OpRd, OpSrc0, SxtName);
}

void AssemblerARM32::sub(const Operand *OpRd, const Operand *OpRn,
                         const Operand *OpSrc1, bool SetFlags,
                         CondARM32::Cond Cond) {
  // SUB (register) - ARM section A8.8.223, encoding A1:
  //   sub{s}<c> <Rd>, <Rn>, <Rm>{, <shift>}
  // SUB (SP minus register): See ARM section 8.8.226, encoding A1:
  //   sub{s}<c> <Rd>, sp, <Rm>{, <Shift>}
  //
  // cccc0000010snnnnddddiiiiitt0mmmm where cccc=Cond, dddd=Rd, nnnn=Rn,
  // mmmm=Rm, iiiii=shift, tt=ShiftKind, and s=SetFlags.
  //
  // Sub (Immediate) - ARM section A8.8.222, encoding A1:
  //    sub{s}<c> <Rd>, <Rn>, #<RotatedImm8>
  // Sub (Sp minus immediate) - ARM section A8.8.225, encoding A1:
  //    sub{s}<c> sp, <Rn>, #<RotatedImm8>
  //
  // cccc0010010snnnnddddiiiiiiiiiiii where cccc=Cond, dddd=Rd, nnnn=Rn,
  // s=SetFlags and iiiiiiiiiiii=Src1Value defining RotatedImm8
  constexpr const char *SubName = "sub";
  constexpr IValueT SubOpcode = B1; // 0010
  emitType01(Cond, SubOpcode, OpRd, OpRn, OpSrc1, SetFlags, RdIsPcAndSetFlags,
             SubName);
}

namespace {

// Use a particular UDF encoding -- TRAPNaCl in LLVM: 0xE7FEDEF0
// http://llvm.org/viewvc/llvm-project?view=revision&revision=173943
const uint8_t TrapBytesRaw[] = {0xE7, 0xFE, 0xDE, 0xF0};

const auto TrapBytes =
    llvm::ArrayRef<uint8_t>(TrapBytesRaw, llvm::array_lengthof(TrapBytesRaw));

} // end of anonymous namespace

llvm::ArrayRef<uint8_t> AssemblerARM32::getNonExecBundlePadding() const {
  return TrapBytes;
}

void AssemblerARM32::trap() {
  AssemblerBuffer::EnsureCapacity ensured(&Buffer);
  for (const uint8_t &Byte : reverse_range(TrapBytes))
    Buffer.emit<uint8_t>(Byte);
}

void AssemblerARM32::tst(const Operand *OpRn, const Operand *OpSrc1,
                         CondARM32::Cond Cond) {
  // TST (register) - ARM section A8.8.241, encoding A1:
  //   tst<c> <Rn>, <Rm>(, <shift>}
  //
  // cccc00010001nnnn0000iiiiitt0mmmm where cccc=Cond, nnnn=Rn, mmmm=Rm,
  // iiiii=Shift, and tt=ShiftKind.
  //
  // TST (immediate) - ARM section A8.8.240, encoding A1:
  //   tst<c> <Rn>, #<RotatedImm8>
  //
  // cccc00110001nnnn0000iiiiiiiiiiii where cccc=Cond, nnnn=Rn, and
  // iiiiiiiiiiii defines RotatedImm8.
  constexpr const char *TstName = "tst";
  constexpr IValueT TstOpcode = B3; // ie. 1000
  emitCompareOp(Cond, TstOpcode, OpRn, OpSrc1, TstName);
}

void AssemblerARM32::udiv(const Operand *OpRd, const Operand *OpRn,
                          const Operand *OpSrc1, CondARM32::Cond Cond) {
  // UDIV - ARM section A8.8.248, encoding A1.
  //   udiv<c> <Rd>, <Rn>, <Rm>
  //
  // cccc01110011dddd1111mmmm0001nnnn where cccc=Cond, dddd=Rd, nnnn=Rn, and
  // mmmm=Rm.
  constexpr const char *UdivName = "udiv";
  IValueT Rd = encodeGPRegister(OpRd, "Rd", UdivName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", UdivName);
  IValueT Rm = encodeGPRegister(OpSrc1, "Rm", UdivName);
  verifyRegNotPc(Rd, "Rd", UdivName);
  verifyRegNotPc(Rn, "Rn", UdivName);
  verifyRegNotPc(Rm, "Rm", UdivName);
  // Assembler registers rd, rn, rm are encoded as rn, rm, rs.
  constexpr IValueT UdivOpcode = B21;
  emitDivOp(Cond, UdivOpcode, Rd, Rn, Rm);
}

void AssemblerARM32::umull(const Operand *OpRdLo, const Operand *OpRdHi,
                           const Operand *OpRn, const Operand *OpRm,
                           CondARM32::Cond Cond) {
  // UMULL - ARM section A8.8.257, encoding A1:
  //   umull<c> <RdLo>, <RdHi>, <Rn>, <Rm>
  //
  // cccc0000100shhhhllllmmmm1001nnnn where hhhh=RdHi, llll=RdLo, nnnn=Rn,
  // mmmm=Rm, and s=SetFlags
  constexpr const char *UmullName = "umull";
  IValueT RdLo = encodeGPRegister(OpRdLo, "RdLo", UmullName);
  IValueT RdHi = encodeGPRegister(OpRdHi, "RdHi", UmullName);
  IValueT Rn = encodeGPRegister(OpRn, "Rn", UmullName);
  IValueT Rm = encodeGPRegister(OpRm, "Rm", UmullName);
  verifyRegNotPc(RdLo, "RdLo", UmullName);
  verifyRegNotPc(RdHi, "RdHi", UmullName);
  verifyRegNotPc(Rn, "Rn", UmullName);
  verifyRegNotPc(Rm, "Rm", UmullName);
  verifyRegsNotEq(RdHi, "RdHi", RdLo, "RdLo", UmullName);
  constexpr IValueT UmullOpcode = B23;
  constexpr bool SetFlags = false;
  emitMulOp(Cond, UmullOpcode, RdLo, RdHi, Rn, Rm, SetFlags);
}

void AssemblerARM32::uxt(const Operand *OpRd, const Operand *OpSrc0,
                         CondARM32::Cond Cond) {
  constexpr const char *UxtName = "uxt";
  constexpr IValueT UxtOpcode = B26 | B25 | B23 | B22 | B21;
  emitSignExtend(Cond, UxtOpcode, OpRd, OpSrc0, UxtName);
}

void AssemblerARM32::vabss(const Operand *OpSd, const Operand *OpSm,
                           CondARM32::Cond Cond) {
  // VABS - ARM section A8.8.280, encoding A2:
  //   vabs<c>.f32 <Sd>, <Sm>
  //
  // cccc11101D110000dddd101011M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vabss = "vabss";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vabss);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vabss);
  constexpr IValueT S0 = 0;
  constexpr IValueT VabssOpcode = B23 | B21 | B20 | B7 | B6;
  emitVFPsss(Cond, VabssOpcode, Sd, S0, Sm);
}

void AssemblerARM32::vabsd(const Operand *OpDd, const Operand *OpDm,
                           CondARM32::Cond Cond) {
  // VABS - ARM section A8.8.280, encoding A2:
  //   vabs<c>.f64 <Dd>, <Dm>
  //
  // cccc11101D110000dddd101111M0mmmm where cccc=Cond, Ddddd=Dd, and Mmmmm=Dm.
  constexpr const char *Vabsd = "vabsd";
  const IValueT Dd = encodeDRegister(OpDd, "Dd", Vabsd);
  const IValueT Dm = encodeDRegister(OpDm, "Dm", Vabsd);
  constexpr IValueT D0 = 0;
  constexpr IValueT VabsdOpcode = B23 | B21 | B20 | B7 | B6;
  emitVFPddd(Cond, VabsdOpcode, Dd, D0, Dm);
}

void AssemblerARM32::vabsq(const Operand *OpQd, const Operand *OpQm) {
  // VABS - ARM section A8.8.280, encoding A1:
  //   vabs.<dt> <Qd>, <Qm>
  //
  // 111100111D11ss01ddd0f1101M0mmm0 where Dddd=OpQd, Mddd=OpQm, and
  // <dt> in {s8, s16, s32, f32} and ss is the encoding of <dt>.
  const Type ElmtTy = typeElementType(OpQd->getType());
  assert(ElmtTy != IceType_i64 && "vabsq doesn't allow i64!");
  constexpr const char *Vabsq = "vabsq";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vabsq));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vabsq));
  constexpr IValueT Dn = 0;
  const IValueT VabsqOpcode =
      B24 | B23 | B21 | B20 | B16 | B9 | B8 | (encodeElmtType(ElmtTy) << 18);
  constexpr bool UseQRegs = true;
  emitSIMDBase(VabsqOpcode, Dd, Dn, Dm, UseQRegs, isFloatingType(ElmtTy));
}

void AssemblerARM32::vadds(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VADD (floating-point) - ARM section A8.8.283, encoding A2:
  //   vadd<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11100D11nnnndddd101sN0M0mmmm where cccc=Cond, s=0, ddddD=Rd, nnnnN=Rn,
  // and mmmmM=Rm.
  constexpr const char *Vadds = "vadds";
  constexpr IValueT VaddsOpcode = B21 | B20;
  emitVFPsss(Cond, VaddsOpcode, OpSd, OpSn, OpSm, Vadds);
}

void AssemblerARM32::vaddqi(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // VADD (integer) - ARM section A8.8.282, encoding A1:
  //   vadd.<dt> <Qd>, <Qn>, <Qm>
  //
  // 111100100Dssnnn0ddd01000N1M0mmm0 where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // and dt in [i8, i16, i32, i64] where ss is the index.
  assert(isScalarIntegerType(ElmtTy) &&
         "vaddqi expects vector with integer element type");
  constexpr const char *Vaddqi = "vaddqi";
  constexpr IValueT VaddqiOpcode = B11;
  emitSIMDqqq(VaddqiOpcode, ElmtTy, OpQd, OpQm, OpQn, Vaddqi);
}

void AssemblerARM32::vaddqf(const Operand *OpQd, const Operand *OpQn,
                            const Operand *OpQm) {
  // VADD (floating-point) - ARM section A8.8.283, Encoding A1:
  //   vadd.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100100D00nnn0ddd01101N1M0mmm0 where Dddd=Qd, Nnnn=Qn, and Mmmm=Qm.
  assert(OpQd->getType() == IceType_v4f32 && "vaddqf expects type <4 x float>");
  constexpr const char *Vaddqf = "vaddqf";
  constexpr IValueT VaddqfOpcode = B11 | B8;
  constexpr bool IsFloatTy = true;
  emitSIMDqqqBase(VaddqfOpcode, OpQd, OpQn, OpQm, IsFloatTy, Vaddqf);
}

void AssemblerARM32::vaddd(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VADD (floating-point) - ARM section A8.8.283, encoding A2:
  //   vadd<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11100D11nnnndddd101sN0M0mmmm where cccc=Cond, s=1, Ddddd=Rd, Nnnnn=Rn,
  // and Mmmmm=Rm.
  constexpr const char *Vaddd = "vaddd";
  constexpr IValueT VadddOpcode = B21 | B20;
  emitVFPddd(Cond, VadddOpcode, OpDd, OpDn, OpDm, Vaddd);
}

void AssemblerARM32::vandq(const Operand *OpQd, const Operand *OpQm,
                           const Operand *OpQn) {
  // VAND (register) - ARM section A8.8.287, encoding A1:
  //   vand <Qd>, <Qn>, <Qm>
  //
  // 111100100D00nnn0ddd00001N1M1mmm0 where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vandq = "vandq";
  constexpr IValueT VandqOpcode = B8 | B4;
  constexpr Type ElmtTy = IceType_i8;
  emitSIMDqqq(VandqOpcode, ElmtTy, OpQd, OpQm, OpQn, Vandq);
}

void AssemblerARM32::vbslq(const Operand *OpQd, const Operand *OpQm,
                           const Operand *OpQn) {
  // VBSL (register) - ARM section A8.8.290, encoding A1:
  //   vbsl <Qd>, <Qn>, <Qm>
  //
  // 111100110D01nnn0ddd00001N1M1mmm0 where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vbslq = "vbslq";
  constexpr IValueT VbslqOpcode = B24 | B20 | B8 | B4;
  constexpr Type ElmtTy = IceType_i8; // emits sz=0
  emitSIMDqqq(VbslqOpcode, ElmtTy, OpQd, OpQm, OpQn, Vbslq);
}

void AssemblerARM32::vceqqi(const Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // vceq (register) - ARM section A8.8.291, encoding A1:
  //   vceq.<st> <Qd>, <Qn>, <Qm>
  //
  // 111100110Dssnnnndddd1000NQM1mmmm where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm, and
  // st in [i8, i16, i32] where ss is the index.
  constexpr const char *Vceq = "vceq";
  constexpr IValueT VceqOpcode = B24 | B11 | B4;
  emitSIMDqqq(VceqOpcode, ElmtTy, OpQd, OpQm, OpQn, Vceq);
}

void AssemblerARM32::vceqqs(const Operand *OpQd, const Operand *OpQm,
                            const Operand *OpQn) {
  // vceq (register) - ARM section A8.8.291, encoding A2:
  //   vceq.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100100D00nnnndddd1110NQM0mmmm where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vceq = "vceq";
  constexpr IValueT VceqOpcode = B11 | B10 | B9;
  constexpr Type ElmtTy = IceType_i8; // encoded as 0b00
  emitSIMDqqq(VceqOpcode, ElmtTy, OpQd, OpQm, OpQn, Vceq);
}

void AssemblerARM32::vcgeqi(const Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // vcge (register) - ARM section A8.8.293, encoding A1:
  //   vcge.<st> <Qd>, <Qn>, <Qm>
  //
  // 1111001U0Dssnnnndddd0011NQM1mmmm where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // 0=U, and st in [s8, s16, s32] where ss is the index.
  constexpr const char *Vcge = "vcge";
  constexpr IValueT VcgeOpcode = B9 | B8 | B4;
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcugeqi(const Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // vcge (register) - ARM section A8.8.293, encoding A1:
  //   vcge.<st> <Qd>, <Qn>, <Qm>
  //
  // 1111001U0Dssnnnndddd0011NQM1mmmm where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // 1=U, and st in [u8, u16, u32] where ss is the index.
  constexpr const char *Vcge = "vcge";
  constexpr IValueT VcgeOpcode = B24 | B9 | B8 | B4;
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcgeqs(const Operand *OpQd, const Operand *OpQm,
                            const Operand *OpQn) {
  // vcge (register) - ARM section A8.8.293, encoding A2:
  //   vcge.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100110D00nnnndddd1110NQM0mmmm where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vcge = "vcge";
  constexpr IValueT VcgeOpcode = B24 | B11 | B10 | B9;
  constexpr Type ElmtTy = IceType_i8; // encoded as 0b00.
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcgtqi(const Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // vcgt (register) - ARM section A8.8.295, encoding A1:
  //   vcgt.<st> <Qd>, <Qn>, <Qm>
  //
  // 1111001U0Dssnnnndddd0011NQM0mmmm where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // 0=U, and st in [s8, s16, s32] where ss is the index.
  constexpr const char *Vcge = "vcgt";
  constexpr IValueT VcgeOpcode = B9 | B8;
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcugtqi(const Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // vcgt (register) - ARM section A8.8.295, encoding A1:
  //   vcgt.<st> <Qd>, <Qn>, <Qm>
  //
  // 111100110Dssnnnndddd0011NQM0mmmm where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // 1=U, and st in [u8, u16, u32] where ss is the index.
  constexpr const char *Vcge = "vcgt";
  constexpr IValueT VcgeOpcode = B24 | B9 | B8;
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcgtqs(const Operand *OpQd, const Operand *OpQm,
                            const Operand *OpQn) {
  // vcgt (register) - ARM section A8.8.295, encoding A2:
  //   vcgt.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100110D10nnnndddd1110NQM0mmmm where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vcge = "vcgt";
  constexpr IValueT VcgeOpcode = B24 | B21 | B11 | B10 | B9;
  constexpr Type ElmtTy = IceType_i8; // encoded as 0b00.
  emitSIMDqqq(VcgeOpcode, ElmtTy, OpQd, OpQm, OpQn, Vcge);
}

void AssemblerARM32::vcmpd(const Operand *OpDd, const Operand *OpDm,
                           CondARM32::Cond Cond) {
  constexpr const char *Vcmpd = "vcmpd";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vcmpd);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vcmpd);
  constexpr IValueT VcmpdOpcode = B23 | B21 | B20 | B18 | B6;
  constexpr IValueT Dn = 0;
  emitVFPddd(Cond, VcmpdOpcode, Dd, Dn, Dm);
}

void AssemblerARM32::vcmpdz(const Operand *OpDd, CondARM32::Cond Cond) {
  constexpr const char *Vcmpdz = "vcmpdz";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vcmpdz);
  constexpr IValueT VcmpdzOpcode = B23 | B21 | B20 | B18 | B16 | B6;
  constexpr IValueT Dn = 0;
  constexpr IValueT Dm = 0;
  emitVFPddd(Cond, VcmpdzOpcode, Dd, Dn, Dm);
}

void AssemblerARM32::vcmps(const Operand *OpSd, const Operand *OpSm,
                           CondARM32::Cond Cond) {
  constexpr const char *Vcmps = "vcmps";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcmps);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcmps);
  constexpr IValueT VcmpsOpcode = B23 | B21 | B20 | B18 | B6;
  constexpr IValueT Sn = 0;
  emitVFPsss(Cond, VcmpsOpcode, Sd, Sn, Sm);
}

void AssemblerARM32::vcmpsz(const Operand *OpSd, CondARM32::Cond Cond) {
  constexpr const char *Vcmpsz = "vcmps";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcmpsz);
  constexpr IValueT VcmpszOpcode = B23 | B21 | B20 | B18 | B16 | B6;
  constexpr IValueT Sn = 0;
  constexpr IValueT Sm = 0;
  emitVFPsss(Cond, VcmpszOpcode, Sd, Sn, Sm);
}

void AssemblerARM32::emitVFPsd(CondARM32::Cond Cond, IValueT Opcode, IValueT Sd,
                               IValueT Dm) {
  assert(Sd < RegARM32::getNumSRegs());
  assert(Dm < RegARM32::getNumDRegs());
  assert(CondARM32::isDefined(Cond));
  constexpr IValueT VFPOpcode = B27 | B26 | B25 | B11 | B9;
  const IValueT Encoding =
      Opcode | VFPOpcode | (encodeCondition(Cond) << kConditionShift) |
      (getYInRegXXXXY(Sd) << 22) | (getXXXXInRegXXXXY(Sd) << 12) |
      (getYInRegYXXXX(Dm) << 5) | getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::vcvtdi(const Operand *OpDd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.f64.s32 <Dd>, <Sm>
  //
  // cccc11101D111000dddd10111M0mmmm where cccc=Cond, Ddddd=Dd, and mmmmM=Sm.
  constexpr const char *Vcvtdi = "vcvtdi";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vcvtdi);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtdi);
  constexpr IValueT VcvtdiOpcode = B23 | B21 | B20 | B19 | B8 | B7 | B6;
  emitVFPds(Cond, VcvtdiOpcode, Dd, Sm);
}

void AssemblerARM32::vcvtdu(const Operand *OpDd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.f64.u32 <Dd>, <Sm>
  //
  // cccc11101D111000dddd10101M0mmmm where cccc=Cond, Ddddd=Dd, and mmmmM=Sm.
  constexpr const char *Vcvtdu = "vcvtdu";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vcvtdu);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtdu);
  constexpr IValueT VcvtduOpcode = B23 | B21 | B20 | B19 | B8 | B6;
  emitVFPds(Cond, VcvtduOpcode, Dd, Sm);
}

void AssemblerARM32::vcvtsd(const Operand *OpSd, const Operand *OpDm,
                            CondARM32::Cond Cond) {
  constexpr const char *Vcvtsd = "vcvtsd";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtsd);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vcvtsd);
  constexpr IValueT VcvtsdOpcode =
      B23 | B21 | B20 | B18 | B17 | B16 | B8 | B7 | B6;
  emitVFPsd(Cond, VcvtsdOpcode, Sd, Dm);
}

void AssemblerARM32::vcvtis(const Operand *OpSd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.s32.f32 <Sd>, <Sm>
  //
  // cccc11101D111101dddd10011M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vcvtis = "vcvtis";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtis);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtis);
  constexpr IValueT VcvtisOpcode = B23 | B21 | B20 | B19 | B18 | B16 | B7 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VcvtisOpcode, Sd, S0, Sm);
}

void AssemblerARM32::vcvtid(const Operand *OpSd, const Operand *OpDm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.s32.f64 <Sd>, <Dm>
  //
  // cccc11101D111101dddd10111M0mmmm where cccc=Cond, ddddD=Sd, and Mmmmm=Dm.
  constexpr const char *Vcvtid = "vcvtid";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtid);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vcvtid);
  constexpr IValueT VcvtidOpcode =
      B23 | B21 | B20 | B19 | B18 | B16 | B8 | B7 | B6;
  emitVFPsd(Cond, VcvtidOpcode, Sd, Dm);
}

void AssemblerARM32::vcvtsi(const Operand *OpSd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.f32.s32 <Sd>, <Sm>
  //
  // cccc11101D111000dddd10011M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vcvtsi = "vcvtsi";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtsi);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtsi);
  constexpr IValueT VcvtsiOpcode = B23 | B21 | B20 | B19 | B7 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VcvtsiOpcode, Sd, S0, Sm);
}

void AssemblerARM32::vcvtsu(const Operand *OpSd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.f32.u32 <Sd>, <Sm>
  //
  // cccc11101D111000dddd10001M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vcvtsu = "vcvtsu";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtsu);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtsu);
  constexpr IValueT VcvtsuOpcode = B23 | B21 | B20 | B19 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VcvtsuOpcode, Sd, S0, Sm);
}

void AssemblerARM32::vcvtud(const Operand *OpSd, const Operand *OpDm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.u32.f64 <Sd>, <Dm>
  //
  // cccc11101D111100dddd10111M0mmmm where cccc=Cond, ddddD=Sd, and Mmmmm=Dm.
  constexpr const char *Vcvtud = "vcvtud";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtud);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vcvtud);
  constexpr IValueT VcvtudOpcode = B23 | B21 | B20 | B19 | B18 | B8 | B7 | B6;
  emitVFPsd(Cond, VcvtudOpcode, Sd, Dm);
}

void AssemblerARM32::vcvtus(const Operand *OpSd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VCVT (between floating-point and integer, Floating-point)
  //      - ARM Section A8.8.306, encoding A1:
  //   vcvt<c>.u32.f32 <Sd>, <Sm>
  //
  // cccc11101D111100dddd10011M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vcvtus = "vcvtus";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vcvtus);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtus);
  constexpr IValueT VcvtsiOpcode = B23 | B21 | B20 | B19 | B18 | B7 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VcvtsiOpcode, Sd, S0, Sm);
}

void AssemblerARM32::vcvtqsi(const Operand *OpQd, const Operand *OpQm) {
  // VCVT (between floating-point and integer, Advanced SIMD)
  //      - ARM Section A8.8.305, encoding A1:
  //   vcvt<c>.f32.s32 <Qd>, <Qm>
  //
  // 111100111D11ss11dddd011ooQM0mmmm where Ddddd=Qd, Mmmmm=Qm, and 10=op.
  constexpr const char *Vcvtqsi = "vcvt.s32.f32";
  constexpr IValueT VcvtqsiOpcode = B8;
  emitSIMDCvtqq(VcvtqsiOpcode, OpQd, OpQm, Vcvtqsi);
}

void AssemblerARM32::vcvtqsu(const Operand *OpQd, const Operand *OpQm) {
  // VCVT (between floating-point and integer, Advanced SIMD)
  //      - ARM Section A8.8.305, encoding A1:
  //   vcvt<c>.f32.u32 <Qd>, <Qm>
  //
  // 111100111D11ss11dddd011ooQM0mmmm where Ddddd=Qd, Mmmmm=Qm, and 11=op.
  constexpr const char *Vcvtqsu = "vcvt.u32.f32";
  constexpr IValueT VcvtqsuOpcode = B8 | B7;
  emitSIMDCvtqq(VcvtqsuOpcode, OpQd, OpQm, Vcvtqsu);
}

void AssemblerARM32::vcvtqis(const Operand *OpQd, const Operand *OpQm) {
  // VCVT (between floating-point and integer, Advanced SIMD)
  //      - ARM Section A8.8.305, encoding A1:
  //   vcvt<c>.f32.s32 <Qd>, <Qm>
  //
  // 111100111D11ss11dddd011ooQM0mmmm where Ddddd=Qd, Mmmmm=Qm, and 01=op.
  constexpr const char *Vcvtqis = "vcvt.f32.s32";
  constexpr IValueT VcvtqisOpcode = 0;
  emitSIMDCvtqq(VcvtqisOpcode, OpQd, OpQm, Vcvtqis);
}

void AssemblerARM32::vcvtqus(const Operand *OpQd, const Operand *OpQm) {
  // VCVT (between floating-point and integer, Advanced SIMD)
  //      - ARM Section A8.8.305, encoding A1:
  //   vcvt<c>.f32.u32 <Qd>, <Qm>
  //
  // 111100111D11ss11dddd011ooQM0mmmm where Ddddd=Qd, Mmmmm=Qm, and 01=op.
  constexpr const char *Vcvtqus = "vcvt.f32.u32";
  constexpr IValueT VcvtqusOpcode = B7;
  emitSIMDCvtqq(VcvtqusOpcode, OpQd, OpQm, Vcvtqus);
}

void AssemblerARM32::emitVFPds(CondARM32::Cond Cond, IValueT Opcode, IValueT Dd,
                               IValueT Sm) {
  assert(Dd < RegARM32::getNumDRegs());
  assert(Sm < RegARM32::getNumSRegs());
  assert(CondARM32::isDefined(Cond));
  constexpr IValueT VFPOpcode = B27 | B26 | B25 | B11 | B9;
  const IValueT Encoding =
      Opcode | VFPOpcode | (encodeCondition(Cond) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (getXXXXInRegYXXXX(Dd) << 12) |
      (getYInRegXXXXY(Sm) << 5) | getXXXXInRegXXXXY(Sm);
  emitInst(Encoding);
}

void AssemblerARM32::vcvtds(const Operand *OpDd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  constexpr const char *Vcvtds = "Vctds";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vcvtds);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vcvtds);
  constexpr IValueT VcvtdsOpcode = B23 | B21 | B20 | B18 | B17 | B16 | B7 | B6;
  emitVFPds(Cond, VcvtdsOpcode, Dd, Sm);
}

void AssemblerARM32::vdivs(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VDIV (floating-point) - ARM section A8.8.283, encoding A2:
  //   vdiv<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11101D00nnnndddd101sN0M0mmmm where cccc=Cond, s=0, ddddD=Rd, nnnnN=Rn,
  // and mmmmM=Rm.
  constexpr const char *Vdivs = "vdivs";
  constexpr IValueT VdivsOpcode = B23;
  emitVFPsss(Cond, VdivsOpcode, OpSd, OpSn, OpSm, Vdivs);
}

void AssemblerARM32::vdivd(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VDIV (floating-point) - ARM section A8.8.283, encoding A2:
  //   vdiv<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11101D00nnnndddd101sN0M0mmmm where cccc=Cond, s=1, Ddddd=Rd, Nnnnn=Rn,
  // and Mmmmm=Rm.
  constexpr const char *Vdivd = "vdivd";
  constexpr IValueT VdivdOpcode = B23;
  emitVFPddd(Cond, VdivdOpcode, OpDd, OpDn, OpDm, Vdivd);
}

void AssemblerARM32::veord(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm) {
  // VEOR - ARM secdtion A8.8.315, encoding A1:
  //   veor<c> <Dd>, <Dn>, <Dm>
  //
  // 111100110D00nnnndddd0001N0M1mmmm where Ddddd=Dd, Nnnnn=Dn, and Mmmmm=Dm.
  constexpr const char *Veord = "veord";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Veord);
  IValueT Dn = encodeDRegister(OpDn, "Dn", Veord);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Veord);
  const IValueT Encoding =
      B25 | B24 | B8 | B4 |
      (encodeCondition(CondARM32::Cond::kNone) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (getXXXXInRegYXXXX(Dn) << 16) |
      (getXXXXInRegYXXXX(Dd) << 12) | (getYInRegYXXXX(Dn) << 7) |
      (getYInRegYXXXX(Dm) << 5) | getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::veorq(const Operand *OpQd, const Operand *OpQn,
                           const Operand *OpQm) {
  // VEOR - ARM section A8.8.316, encoding A1:
  //   veor <Qd>, <Qn>, <Qm>
  //
  // 111100110D00nnn0ddd00001N1M1mmm0 where Dddd=Qd, Nnnn=Qn, and Mmmm=Qm.
  constexpr const char *Veorq = "veorq";
  constexpr IValueT VeorqOpcode = B24 | B8 | B4;
  emitSIMDqqq(VeorqOpcode, IceType_i8, OpQd, OpQn, OpQm, Veorq);
}

void AssemblerARM32::vldrd(const Operand *OpDd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // VLDR - ARM section A8.8.333, encoding A1.
  //   vldr<c> <Dd>, [<Rn>{, #+/-<imm>}]
  //
  // cccc1101UD01nnnndddd1011iiiiiiii where cccc=Cond, nnnn=Rn, Ddddd=Rd,
  // iiiiiiii=abs(Imm >> 2), and U=1 if Opcode>=0.
  constexpr const char *Vldrd = "vldrd";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vldrd);
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  EncodedOperand AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding = B27 | B26 | B24 | B20 | B11 | B9 | B8 |
                     (encodeCondition(Cond) << kConditionShift) |
                     (getYInRegYXXXX(Dd) << 22) |
                     (getXXXXInRegYXXXX(Dd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::vldrq(const Operand *OpQd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // This is a pseudo-instruction which loads 64-bit data into a quadword
  // vector register. It is implemented by loading into the lower doubleword.

  // VLDR - ARM section A8.8.333, encoding A1.
  //   vldr<c> <Dd>, [<Rn>{, #+/-<imm>}]
  //
  // cccc1101UD01nnnndddd1011iiiiiiii where cccc=Cond, nnnn=Rn, Ddddd=Rd,
  // iiiiiiii=abs(Imm >> 2), and U=1 if Opcode>=0.
  constexpr const char *Vldrd = "vldrd";
  IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vldrd));
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  EncodedOperand AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding = B27 | B26 | B24 | B20 | B11 | B9 | B8 |
                     (encodeCondition(Cond) << kConditionShift) |
                     (getYInRegYXXXX(Dd) << 22) |
                     (getXXXXInRegYXXXX(Dd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::vldrs(const Operand *OpSd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // VDLR - ARM section A8.8.333, encoding A2.
  //   vldr<c> <Sd>, [<Rn>{, #+/-<imm>]]
  //
  // cccc1101UD01nnnndddd1010iiiiiiii where cccc=Cond, nnnn=Rn, ddddD=Sd,
  // iiiiiiii=abs(Opcode), and U=1 if Opcode >= 0;
  constexpr const char *Vldrs = "vldrs";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vldrs);
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  EncodedOperand AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding = B27 | B26 | B24 | B20 | B11 | B9 |
                     (encodeCondition(Cond) << kConditionShift) |
                     (getYInRegXXXXY(Sd) << 22) |
                     (getXXXXInRegXXXXY(Sd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::emitVMem1Op(IValueT Opcode, IValueT Dd, IValueT Rn,
                                 IValueT Rm, DRegListSize NumDRegs,
                                 size_t ElmtSize, IValueT Align,
                                 const char *InstName) {
  assert(Utils::IsAbsoluteUint(2, Align));
  IValueT EncodedElmtSize;
  switch (ElmtSize) {
  default: {
    std::string Buffer;
    llvm::raw_string_ostream StrBuf(Buffer);
    StrBuf << InstName << ": found invalid vector element size " << ElmtSize;
    llvm::report_fatal_error(StrBuf.str());
  }
  case 8:
    EncodedElmtSize = 0;
    break;
  case 16:
    EncodedElmtSize = 1;
    break;
  case 32:
    EncodedElmtSize = 2;
    break;
  case 64:
    EncodedElmtSize = 3;
  }
  const IValueT Encoding =
      Opcode | (encodeCondition(CondARM32::kNone) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (Rn << kRnShift) |
      (getXXXXInRegYXXXX(Dd) << kRdShift) | (NumDRegs << 8) |
      (EncodedElmtSize << 6) | (Align << 4) | Rm;
  emitInst(Encoding);
}

void AssemblerARM32::emitVMem1Op(IValueT Opcode, IValueT Dd, IValueT Rn,
                                 IValueT Rm, size_t ElmtSize, IValueT Align,
                                 const char *InstName) {
  assert(Utils::IsAbsoluteUint(2, Align));
  IValueT EncodedElmtSize;
  switch (ElmtSize) {
  default: {
    std::string Buffer;
    llvm::raw_string_ostream StrBuf(Buffer);
    StrBuf << InstName << ": found invalid vector element size " << ElmtSize;
    llvm::report_fatal_error(StrBuf.str());
  }
  case 8:
    EncodedElmtSize = 0;
    break;
  case 16:
    EncodedElmtSize = 1;
    break;
  case 32:
    EncodedElmtSize = 2;
    break;
  case 64:
    EncodedElmtSize = 3;
  }
  const IValueT Encoding =
      Opcode | (encodeCondition(CondARM32::kNone) << kConditionShift) |
      (getYInRegYXXXX(Dd) << 22) | (Rn << kRnShift) |
      (getXXXXInRegYXXXX(Dd) << kRdShift) | (EncodedElmtSize << 10) |
      (Align << 4) | Rm;
  emitInst(Encoding);
}

void AssemblerARM32::vld1qr(size_t ElmtSize, const Operand *OpQd,
                            const Operand *OpAddress, const TargetInfo &TInfo) {
  // VLD1 (multiple single elements) - ARM section A8.8.320, encoding A1:
  //   vld1.<size> <Qd>, [<Rn>]
  //
  // 111101000D10nnnnddd0ttttssaammmm where tttt=DRegListSize2, Dddd=Qd,
  // nnnn=Rn, aa=0 (use default alignment), size=ElmtSize, and ss is the
  // encoding of ElmtSize.
  constexpr const char *Vld1qr = "vld1qr";
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vld1qr);
  const IValueT Dd = mapQRegToDReg(Qd);
  IValueT Address;
  if (encodeAddress(OpAddress, Address, TInfo, NoImmOffsetAddress) !=
      EncodedAsImmRegOffset)
    llvm::report_fatal_error(std::string(Vld1qr) + ": malform memory address");
  const IValueT Rn = mask(Address, kRnShift, 4);
  constexpr IValueT Rm = RegARM32::Reg_pc;
  constexpr IValueT Opcode = B26 | B21;
  constexpr IValueT Align = 0; // use default alignment.
  emitVMem1Op(Opcode, Dd, Rn, Rm, DRegListSize2, ElmtSize, Align, Vld1qr);
}

void AssemblerARM32::vld1(size_t ElmtSize, const Operand *OpQd,
                          const Operand *OpAddress, const TargetInfo &TInfo) {
  // This is a pseudo-instruction for loading a single element of a quadword
  // vector. For 64-bit the lower doubleword vector is loaded.

  if (ElmtSize == 64) {
    return vldrq(OpQd, OpAddress, Ice::CondARM32::AL, TInfo);
  }

  // VLD1 (single elements to one lane) - ARMv7-A/R section A8.6.308, encoding
  // A1:
  //   VLD1<c>.<size> <list>, [<Rn>{@<align>}], <Rm>
  //
  // 111101001D10nnnnddddss00aaaammmm where tttt=DRegListSize2, Dddd=Qd,
  // nnnn=Rn, aa=0 (use default alignment), size=ElmtSize, and ss is the
  // encoding of ElmtSize.
  constexpr const char *Vld1qr = "vld1qr";
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vld1qr);
  const IValueT Dd = mapQRegToDReg(Qd);
  IValueT Address;
  if (encodeAddress(OpAddress, Address, TInfo, NoImmOffsetAddress) !=
      EncodedAsImmRegOffset)
    llvm::report_fatal_error(std::string(Vld1qr) + ": malform memory address");
  const IValueT Rn = mask(Address, kRnShift, 4);
  constexpr IValueT Rm = RegARM32::Reg_pc;
  constexpr IValueT Opcode = B26 | B23 | B21;
  constexpr IValueT Align = 0; // use default alignment.
  emitVMem1Op(Opcode, Dd, Rn, Rm, ElmtSize, Align, Vld1qr);
}

bool AssemblerARM32::vmovqc(const Operand *OpQd, const ConstantInteger32 *Imm) {
  // VMOV (immediate) - ARM section A8.8.320, encoding A1:
  //   VMOV.<dt> <Qd>, #<Imm>
  // 1111001x1D000yyyddddcccc01p1zzzz where Qd=Ddddd, Imm=xyyyzzzz, cmode=cccc,
  // and Op=p.
  constexpr const char *Vmovc = "vmovc";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmovc));
  IValueT Value = Imm->getValue();
  const Type VecTy = OpQd->getType();
  if (!isVectorType(VecTy))
    return false;

  IValueT Op;
  IValueT Cmode;
  IValueT Imm8;
  if (!encodeAdvSIMDExpandImm(Value, typeElementType(VecTy), Op, Cmode, Imm8))
    return false;
  if (Op == 0 && mask(Cmode, 0, 1) == 1)
    return false;
  if (Op == 1 && Cmode != 13)
    return false;
  const IValueT Encoding =
      (0xF << kConditionShift) | B25 | B23 | B6 | B4 |
      (mask(Imm8, 7, 1) << 24) | (getYInRegYXXXX(Dd) << 22) |
      (mask(Imm8, 4, 3) << 16) | (getXXXXInRegYXXXX(Dd) << 12) | (Cmode << 8) |
      (Op << 5) | mask(Imm8, 0, 4);
  emitInst(Encoding);
  return true;
}

void AssemblerARM32::vmovd(const Operand *OpDd,
                           const OperandARM32FlexFpImm *OpFpImm,
                           CondARM32::Cond Cond) {
  // VMOV (immediate) - ARM section A8.8.339, encoding A2:
  //   vmov<c>.f64 <Dd>, #<imm>
  //
  // cccc11101D11xxxxdddd10110000yyyy where cccc=Cond, ddddD=Sn, xxxxyyyy=imm.
  constexpr const char *Vmovd = "vmovd";
  IValueT Dd = encodeSRegister(OpDd, "Dd", Vmovd);
  IValueT Imm8 = OpFpImm->getModifiedImm();
  assert(Imm8 < (1 << 8));
  constexpr IValueT VmovsOpcode = B23 | B21 | B20 | B8;
  IValueT OpcodePlusImm8 = VmovsOpcode | ((Imm8 >> 4) << 16) | (Imm8 & 0xf);
  constexpr IValueT D0 = 0;
  emitVFPddd(Cond, OpcodePlusImm8, Dd, D0, D0);
}

void AssemblerARM32::vmovdd(const Operand *OpDd, const Variable *OpDm,
                            CondARM32::Cond Cond) {
  // VMOV (register) - ARM section A8.8.340, encoding A2:
  //   vmov<c>.f64 <Dd>, <Sm>
  //
  // cccc11101D110000dddd101101M0mmmm where cccc=Cond, Ddddd=Sd, and Mmmmm=Sm.
  constexpr const char *Vmovdd = "Vmovdd";
  IValueT Dd = encodeSRegister(OpDd, "Dd", Vmovdd);
  IValueT Dm = encodeSRegister(OpDm, "Dm", Vmovdd);
  constexpr IValueT VmovddOpcode = B23 | B21 | B20 | B6;
  constexpr IValueT D0 = 0;
  emitVFPddd(Cond, VmovddOpcode, Dd, D0, Dm);
}

void AssemblerARM32::vmovdrr(const Operand *OpDm, const Operand *OpRt,
                             const Operand *OpRt2, CondARM32::Cond Cond) {
  // VMOV (between two ARM core registers and a doubleword extension register).
  // ARM section A8.8.345, encoding A1:
  //   vmov<c> <Dm>, <Rt>, <Rt2>
  //
  // cccc11000100xxxxyyyy101100M1mmmm where cccc=Cond, xxxx=Rt, yyyy=Rt2, and
  // Mmmmm=Dm.
  constexpr const char *Vmovdrr = "vmovdrr";
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vmovdrr);
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Vmovdrr);
  IValueT Rt2 = encodeGPRegister(OpRt2, "Rt", Vmovdrr);
  assert(Rt != RegARM32::Encoded_Reg_sp);
  assert(Rt != RegARM32::Encoded_Reg_pc);
  assert(Rt2 != RegARM32::Encoded_Reg_sp);
  assert(Rt2 != RegARM32::Encoded_Reg_pc);
  assert(Rt != Rt2);
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = B27 | B26 | B22 | B11 | B9 | B8 | B4 |
                     (encodeCondition(Cond) << kConditionShift) | (Rt2 << 16) |
                     (Rt << 12) | (getYInRegYXXXX(Dm) << 5) |
                     getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::vmovqir(const Operand *OpQn, uint32_t Index,
                             const Operand *OpRt, CondARM32::Cond Cond) {
  // VMOV (ARM core register to scalar) - ARM section A8.8.341, encoding A1:
  //   vmov<c>.<size> <Dn[x]>, <Rt>
  constexpr const char *Vmovdr = "vmovdr";
  constexpr bool IsExtract = true;
  emitInsertExtractInt(Cond, OpQn, Index, OpRt, !IsExtract, Vmovdr);
}

void AssemblerARM32::vmovqis(const Operand *OpQd, uint32_t Index,
                             const Operand *OpSm, CondARM32::Cond Cond) {
  constexpr const char *Vmovqis = "vmovqis";
  assert(Index < 4);
  IValueT Sd = mapQRegToSReg(encodeQRegister(OpQd, "Qd", Vmovqis)) + Index;
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vmovqis);
  emitMoveSS(Cond, Sd, Sm);
}

void AssemblerARM32::vmovrqi(const Operand *OpRt, const Operand *OpQn,
                             uint32_t Index, CondARM32::Cond Cond) {
  // VMOV (scalar to ARM core register) - ARM section A8.8.342, encoding A1:
  //   vmov<c>.<dt> <Rt>, <Dn[x]>
  constexpr const char *Vmovrd = "vmovrd";
  constexpr bool IsExtract = true;
  emitInsertExtractInt(Cond, OpQn, Index, OpRt, IsExtract, Vmovrd);
}

void AssemblerARM32::vmovrrd(const Operand *OpRt, const Operand *OpRt2,
                             const Operand *OpDm, CondARM32::Cond Cond) {
  // VMOV (between two ARM core registers and a doubleword extension register).
  // ARM section A8.8.345, encoding A1:
  //   vmov<c> <Rt>, <Rt2>, <Dm>
  //
  // cccc11000101xxxxyyyy101100M1mmmm where cccc=Cond, xxxx=Rt, yyyy=Rt2, and
  // Mmmmm=Dm.
  constexpr const char *Vmovrrd = "vmovrrd";
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Vmovrrd);
  IValueT Rt2 = encodeGPRegister(OpRt2, "Rt", Vmovrrd);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vmovrrd);
  assert(Rt != RegARM32::Encoded_Reg_sp);
  assert(Rt != RegARM32::Encoded_Reg_pc);
  assert(Rt2 != RegARM32::Encoded_Reg_sp);
  assert(Rt2 != RegARM32::Encoded_Reg_pc);
  assert(Rt != Rt2);
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = B27 | B26 | B22 | B20 | B11 | B9 | B8 | B4 |
                     (encodeCondition(Cond) << kConditionShift) | (Rt2 << 16) |
                     (Rt << 12) | (getYInRegYXXXX(Dm) << 5) |
                     getXXXXInRegYXXXX(Dm);
  emitInst(Encoding);
}

void AssemblerARM32::vmovrs(const Operand *OpRt, const Operand *OpSn,
                            CondARM32::Cond Cond) {
  // VMOV (between ARM core register and single-precision register)
  //   ARM section A8.8.343, encoding A1.
  //
  //   vmov<c> <Rt>, <Sn>
  //
  // cccc11100001nnnntttt1010N0010000 where cccc=Cond, nnnnN = Sn, and tttt=Rt.
  constexpr const char *Vmovrs = "vmovrs";
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Vmovrs);
  IValueT Sn = encodeSRegister(OpSn, "Sn", Vmovrs);
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | B27 | B26 |
                     B25 | B20 | B11 | B9 | B4 | (getXXXXInRegXXXXY(Sn) << 16) |
                     (Rt << kRdShift) | (getYInRegXXXXY(Sn) << 7);
  emitInst(Encoding);
}

void AssemblerARM32::vmovs(const Operand *OpSd,
                           const OperandARM32FlexFpImm *OpFpImm,
                           CondARM32::Cond Cond) {
  // VMOV (immediate) - ARM section A8.8.339, encoding A2:
  //   vmov<c>.f32 <Sd>, #<imm>
  //
  // cccc11101D11xxxxdddd10100000yyyy where cccc=Cond, ddddD=Sn, xxxxyyyy=imm.
  constexpr const char *Vmovs = "vmovs";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vmovs);
  IValueT Imm8 = OpFpImm->getModifiedImm();
  assert(Imm8 < (1 << 8));
  constexpr IValueT VmovsOpcode = B23 | B21 | B20;
  IValueT OpcodePlusImm8 = VmovsOpcode | ((Imm8 >> 4) << 16) | (Imm8 & 0xf);
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, OpcodePlusImm8, Sd, S0, S0);
}

void AssemblerARM32::vmovss(const Operand *OpSd, const Variable *OpSm,
                            CondARM32::Cond Cond) {
  constexpr const char *Vmovss = "Vmovss";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vmovss);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vmovss);
  emitMoveSS(Cond, Sd, Sm);
}

void AssemblerARM32::vmovsqi(const Operand *OpSd, const Operand *OpQm,
                             uint32_t Index, CondARM32::Cond Cond) {
  constexpr const char *Vmovsqi = "vmovsqi";
  const IValueT Sd = encodeSRegister(OpSd, "Sd", Vmovsqi);
  assert(Index < 4);
  const IValueT Sm =
      mapQRegToSReg(encodeQRegister(OpQm, "Qm", Vmovsqi)) + Index;
  emitMoveSS(Cond, Sd, Sm);
}

void AssemblerARM32::vmovsr(const Operand *OpSn, const Operand *OpRt,
                            CondARM32::Cond Cond) {
  // VMOV (between ARM core register and single-precision register)
  //   ARM section A8.8.343, encoding A1.
  //
  //   vmov<c> <Sn>, <Rt>
  //
  // cccc11100000nnnntttt1010N0010000 where cccc=Cond, nnnnN = Sn, and tttt=Rt.
  constexpr const char *Vmovsr = "vmovsr";
  IValueT Sn = encodeSRegister(OpSn, "Sn", Vmovsr);
  IValueT Rt = encodeGPRegister(OpRt, "Rt", Vmovsr);
  assert(Sn < RegARM32::getNumSRegs());
  assert(Rt < RegARM32::getNumGPRegs());
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = (encodeCondition(Cond) << kConditionShift) | B27 | B26 |
                     B25 | B11 | B9 | B4 | (getXXXXInRegXXXXY(Sn) << 16) |
                     (Rt << kRdShift) | (getYInRegXXXXY(Sn) << 7);
  emitInst(Encoding);
}

void AssemblerARM32::vmlad(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VMLA, VMLS (floating-point), ARM section A8.8.337, encoding A2:
  //   vmla<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11100d00nnnndddd1011n0M0mmmm where cccc=Cond, Ddddd=Dd, Nnnnn=Dn, and
  // Mmmmm=Dm
  constexpr const char *Vmlad = "vmlad";
  constexpr IValueT VmladOpcode = 0;
  emitVFPddd(Cond, VmladOpcode, OpDd, OpDn, OpDm, Vmlad);
}

void AssemblerARM32::vmlas(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VMLA, VMLS (floating-point), ARM section A8.8.337, encoding A2:
  //   vmla<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11100d00nnnndddd1010n0M0mmmm where cccc=Cond, ddddD=Sd, nnnnN=Sn, and
  // mmmmM=Sm
  constexpr const char *Vmlas = "vmlas";
  constexpr IValueT VmlasOpcode = 0;
  emitVFPsss(Cond, VmlasOpcode, OpSd, OpSn, OpSm, Vmlas);
}

void AssemblerARM32::vmlsd(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VMLA, VMLS (floating-point), ARM section A8.8.337, encoding A2:
  //   vmls<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11100d00nnnndddd1011n1M0mmmm where cccc=Cond, Ddddd=Dd, Nnnnn=Dn, and
  // Mmmmm=Dm
  constexpr const char *Vmlad = "vmlad";
  constexpr IValueT VmladOpcode = B6;
  emitVFPddd(Cond, VmladOpcode, OpDd, OpDn, OpDm, Vmlad);
}

void AssemblerARM32::vmlss(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VMLA, VMLS (floating-point), ARM section A8.8.337, encoding A2:
  //   vmls<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11100d00nnnndddd1010n1M0mmmm where cccc=Cond, ddddD=Sd, nnnnN=Sn, and
  // mmmmM=Sm
  constexpr const char *Vmlas = "vmlas";
  constexpr IValueT VmlasOpcode = B6;
  emitVFPsss(Cond, VmlasOpcode, OpSd, OpSn, OpSm, Vmlas);
}

void AssemblerARM32::vmrsAPSR_nzcv(CondARM32::Cond Cond) {
  // MVRS - ARM section A*.8.348, encoding A1:
  //   vmrs<c> APSR_nzcv, FPSCR
  //
  // cccc111011110001tttt101000010000 where tttt=0x15 (i.e. when Rt=pc, use
  // APSR_nzcv instead).
  assert(CondARM32::isDefined(Cond));
  IValueT Encoding = B27 | B26 | B25 | B23 | B22 | B21 | B20 | B16 | B15 | B14 |
                     B13 | B12 | B11 | B9 | B4 |
                     (encodeCondition(Cond) << kConditionShift);
  emitInst(Encoding);
}

void AssemblerARM32::vmuls(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VMUL (floating-point) - ARM section A8.8.351, encoding A2:
  //   vmul<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11100D10nnnndddd101sN0M0mmmm where cccc=Cond, s=0, ddddD=Rd, nnnnN=Rn,
  // and mmmmM=Rm.
  constexpr const char *Vmuls = "vmuls";
  constexpr IValueT VmulsOpcode = B21;
  emitVFPsss(Cond, VmulsOpcode, OpSd, OpSn, OpSm, Vmuls);
}

void AssemblerARM32::vmuld(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VMUL (floating-point) - ARM section A8.8.351, encoding A2:
  //   vmul<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11100D10nnnndddd101sN0M0mmmm where cccc=Cond, s=1, Ddddd=Rd, Nnnnn=Rn,
  // and Mmmmm=Rm.
  constexpr const char *Vmuld = "vmuld";
  constexpr IValueT VmuldOpcode = B21;
  emitVFPddd(Cond, VmuldOpcode, OpDd, OpDn, OpDm, Vmuld);
}

void AssemblerARM32::vmulqi(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQn, const Operand *OpQm) {
  // VMUL, VMULL (integer and polynomial) - ARM section A8.8.350, encoding A1:
  //   vmul<c>.<dt> <Qd>, <Qn>, <Qm>
  //
  // 111100100Dssnnn0ddd01001NqM1mmm0 where Dddd=Qd, Nnnn=Qn, Mmmm=Qm, and
  // dt in [i8, i16, i32] where ss is the index.
  assert(isScalarIntegerType(ElmtTy) &&
         "vmulqi expects vector with integer element type");
  assert(ElmtTy != IceType_i64 && "vmulqi on i64 vector not allowed");
  constexpr const char *Vmulqi = "vmulqi";
  constexpr IValueT VmulqiOpcode = B11 | B8 | B4;
  emitSIMDqqq(VmulqiOpcode, ElmtTy, OpQd, OpQn, OpQm, Vmulqi);
}

void AssemblerARM32::vmulh(Type ElmtTy, const Operand *OpQd,
                           const Operand *OpQn, const Operand *OpQm,
                           bool Unsigned) {
  // Pseudo-instruction for multiplying the corresponding elements in the lower
  // halves of two quadword vectors, and returning the high halves.

  // VMULL (integer and polynomial) - ARMv7-A/R section A8.6.337, encoding A1:
  //   VMUL<c>.<dt> <Dd>, <Dn>, <Dm>
  //
  // 1111001U1Dssnnnndddd11o0N0M0mmmm
  assert(isScalarIntegerType(ElmtTy) &&
         "vmull expects vector with integer element type");
  assert(ElmtTy != IceType_i64 && "vmull on i64 vector not allowed");
  constexpr const char *Vmull = "vmull";

  constexpr IValueT ElmtShift = 20;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));

  const IValueT VmullOpcode =
      B25 | (Unsigned ? B24 : 0) | B23 | (B20) | B11 | B10;

  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vmull);
  const IValueT Qn = encodeQRegister(OpQn, "Qn", Vmull);
  const IValueT Qm = encodeQRegister(OpQm, "Qm", Vmull);

  const IValueT Dd = mapQRegToDReg(Qd);
  const IValueT Dn = mapQRegToDReg(Qn);
  const IValueT Dm = mapQRegToDReg(Qm);

  constexpr bool UseQRegs = false;
  constexpr bool IsFloatTy = false;
  emitSIMDBase(VmullOpcode | (ElmtSize << ElmtShift), Dd, Dn, Dm, UseQRegs,
               IsFloatTy);

  // Shift and narrow to obtain high halves.
  constexpr IValueT VshrnOpcode = B25 | B23 | B11 | B4;
  const IValueT Imm6 = encodeSIMDShiftImm6(ST_Vshr, IceType_i16, 16);
  constexpr IValueT ImmShift = 16;

  emitSIMDBase(VshrnOpcode | (Imm6 << ImmShift), Dd, 0, Dd, UseQRegs,
               IsFloatTy);
}

void AssemblerARM32::vmlap(Type ElmtTy, const Operand *OpQd,
                           const Operand *OpQn, const Operand *OpQm) {
  // Pseudo-instruction for multiplying the corresponding elements in the lower
  // halves of two quadword vectors, and pairwise-adding the results.

  // VMULL (integer and polynomial) - ARM section A8.8.350, encoding A1:
  //   vmull<c>.<dt> <Qd>, <Qn>, <Qm>
  //
  // 1111001U1Dssnnnndddd11o0N0M0mmmm
  assert(isScalarIntegerType(ElmtTy) &&
         "vmull expects vector with integer element type");
  assert(ElmtTy != IceType_i64 && "vmull on i64 vector not allowed");
  constexpr const char *Vmull = "vmull";

  constexpr IValueT ElmtShift = 20;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));

  bool Unsigned = false;
  const IValueT VmullOpcode =
      B25 | (Unsigned ? B24 : 0) | B23 | (B20) | B11 | B10;

  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmull));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vmull));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vmull));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloatTy = false;
  emitSIMDBase(VmullOpcode | (ElmtSize << ElmtShift), Dd, Dn, Dm, UseQRegs,
               IsFloatTy);

  // VPADD - ARM section A8.8.280, encoding A1:
  //   vpadd.<dt> <Dd>, <Dm>, <Dn>
  //
  // 111100100Dssnnnndddd1011NQM1mmmm where Ddddd=<Dd>, Mmmmm=<Dm>, and
  // Nnnnn=<Dn> and ss is the encoding of <dt>.
  assert(ElmtTy != IceType_i64 && "vpadd doesn't allow i64!");
  const IValueT VpaddOpcode =
      B25 | B11 | B9 | B8 | B4 | ((encodeElmtType(ElmtTy) + 1) << 20);
  emitSIMDBase(VpaddOpcode, Dd, Dd, Dd + 1, UseQRegs, IsFloatTy);
}

void AssemblerARM32::vdup(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
                          IValueT Idx) {
  // VDUP (scalar) - ARMv7-A/R section A8.6.302, encoding A1:
  //   VDUP<c>.<size> <Qd>, <Dm[x]>
  //
  // 111100111D11iiiiddd011000QM0mmmm where Dddd=<Qd>, Mmmmm=<Dm>, and
  // iiii=imm4 encodes <size> and [x].
  constexpr const char *Vdup = "vdup";

  const IValueT VdupOpcode = B25 | B24 | B23 | B21 | B20 | B11 | B10;

  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vdup));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vdup));

  constexpr bool UseQRegs = true;
  constexpr bool IsFloatTy = false;

  IValueT Imm4 = 0;
  bool Lower = true;
  switch (ElmtTy) {
  case IceType_i8:
    assert(Idx < 16);
    Lower = Idx < 8;
    Imm4 = 1 | ((Idx & 0x7) << 1);
    break;
  case IceType_i16:
    assert(Idx < 8);
    Lower = Idx < 4;
    Imm4 = 2 | ((Idx & 0x3) << 2);
    break;
  case IceType_i32:
  case IceType_f32:
    assert(Idx < 4);
    Lower = Idx < 2;
    Imm4 = 4 | ((Idx & 0x1) << 3);
    break;
  default:
    assert(false && "vdup only supports 8, 16, and 32-bit elements");
    break;
  }

  emitSIMDBase(VdupOpcode, Dd, Imm4, Dn + (Lower ? 0 : 1), UseQRegs, IsFloatTy);
}

void AssemblerARM32::vzip(Type ElmtTy, const Operand *OpQd, const Operand *OpQn,
                          const Operand *OpQm) {
  // Pseudo-instruction which interleaves the elements of the lower halves of
  // two quadword registers.

  // Vzip - ARMv7-A/R section A8.6.410, encoding A1:
  //   VZIP<c>.<size> <Dd>, <Dm>
  //
  // 111100111D11ss10dddd00011QM0mmmm where Ddddd=<Dd>, Mmmmm=<Dm>, and
  // ss=<size>
  assert(ElmtTy != IceType_i64 && "vzip on i64 vector not allowed");

  constexpr const char *Vzip = "vzip";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vzip));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vzip));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vzip));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloatTy = false;

  // VMOV Dd, Dm
  // 111100100D10mmmmdddd0001MQM1mmmm
  constexpr IValueT VmovOpcode = B25 | B21 | B8 | B4;

  // Copy lower half of second source to upper half of destination.
  emitSIMDBase(VmovOpcode, Dd + 1, Dm, Dm, UseQRegs, IsFloatTy);

  // Copy lower half of first source to lower half of destination.
  if (Dd != Dn)
    emitSIMDBase(VmovOpcode, Dd, Dn, Dn, UseQRegs, IsFloatTy);

  constexpr IValueT ElmtShift = 18;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));

  if (ElmtTy != IceType_i32 && ElmtTy != IceType_f32) {
    constexpr IValueT VzipOpcode = B25 | B24 | B23 | B21 | B20 | B17 | B8 | B7;
    // Zip the lower and upper half of destination.
    emitSIMDBase(VzipOpcode | (ElmtSize << ElmtShift), Dd, 0, Dd + 1, UseQRegs,
                 IsFloatTy);
  } else {
    constexpr IValueT VtrnOpcode = B25 | B24 | B23 | B21 | B20 | B17 | B7;
    emitSIMDBase(VtrnOpcode | (ElmtSize << ElmtShift), Dd, 0, Dd + 1, UseQRegs,
                 IsFloatTy);
  }
}

void AssemblerARM32::vmulqf(const Operand *OpQd, const Operand *OpQn,
                            const Operand *OpQm) {
  // VMUL (floating-point) - ARM section A8.8.351, encoding A1:
  //   vmul.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100110D00nnn0ddd01101MqM1mmm0 where Dddd=Qd, Nnnn=Qn, and Mmmm=Qm.
  assert(OpQd->getType() == IceType_v4f32 && "vmulqf expects type <4 x float>");
  constexpr const char *Vmulqf = "vmulqf";
  constexpr IValueT VmulqfOpcode = B24 | B11 | B8 | B4;
  constexpr bool IsFloatTy = true;
  emitSIMDqqqBase(VmulqfOpcode, OpQd, OpQn, OpQm, IsFloatTy, Vmulqf);
}

void AssemblerARM32::vmvnq(const Operand *OpQd, const Operand *OpQm) {
  // VMVN (integer) - ARM section A8.8.354, encoding A1:
  //   vmvn <Qd>, <Qm>
  //
  // 111100111D110000dddd01011QM0mmmm where Dddd=Qd, Mmmm=Qm, and 1=Q.
  // TODO(jpp) xxx: unify
  constexpr const char *Vmvn = "vmvn";
  constexpr IValueT VmvnOpcode = B24 | B23 | B21 | B20 | B10 | B8 | B7;
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vmvn);
  constexpr IValueT Qn = 0;
  const IValueT Qm = encodeQRegister(OpQm, "Qm", Vmvn);
  constexpr bool UseQRegs = true;
  constexpr bool IsFloat = false;
  emitSIMDBase(VmvnOpcode, mapQRegToDReg(Qd), mapQRegToDReg(Qn),
               mapQRegToDReg(Qm), UseQRegs, IsFloat);
}

void AssemblerARM32::vmovlq(const Operand *OpQd, const Operand *OpQn,
                            const Operand *OpQm) {
  // Pseudo-instruction to copy the first source operand and insert the lower
  // half of the second operand into the lower half of the destination.

  // VMOV (register) - ARMv7-A/R section A8.6.327, encoding A1:
  //   VMOV<c> <Dd>, <Dm>
  //
  // 111100111D110000ddd001011QM0mmm0 where Dddd=Qd, Mmmm=Qm, and Q=0.

  constexpr const char *Vmov = "vmov";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmov));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vmov));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vmov));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloat = false;

  const IValueT VmovOpcode = B25 | B21 | B8 | B4;

  if (Dd != Dm)
    emitSIMDBase(VmovOpcode, Dd, Dm, Dm, UseQRegs, IsFloat);
  if (Dd + 1 != Dn + 1)
    emitSIMDBase(VmovOpcode, Dd + 1, Dn + 1, Dn + 1, UseQRegs, IsFloat);
}

void AssemblerARM32::vmovhq(const Operand *OpQd, const Operand *OpQn,
                            const Operand *OpQm) {
  // Pseudo-instruction to copy the first source operand and insert the high
  // half of the second operand into the high half of the destination.

  // VMOV (register) - ARMv7-A/R section A8.6.327, encoding A1:
  //   VMOV<c> <Dd>, <Dm>
  //
  // 111100111D110000ddd001011QM0mmm0 where Dddd=Qd, Mmmm=Qm, and Q=0.

  constexpr const char *Vmov = "vmov";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmov));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vmov));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vmov));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloat = false;

  const IValueT VmovOpcode = B25 | B21 | B8 | B4;

  if (Dd != Dn)
    emitSIMDBase(VmovOpcode, Dd, Dn, Dn, UseQRegs, IsFloat);
  if (Dd + 1 != Dm + 1)
    emitSIMDBase(VmovOpcode, Dd + 1, Dm + 1, Dm + 1, UseQRegs, IsFloat);
}

void AssemblerARM32::vmovhlq(const Operand *OpQd, const Operand *OpQn,
                             const Operand *OpQm) {
  // Pseudo-instruction to copy the first source operand and insert the high
  // half of the second operand into the lower half of the destination.

  // VMOV (register) - ARMv7-A/R section A8.6.327, encoding A1:
  //   VMOV<c> <Dd>, <Dm>
  //
  // 111100111D110000ddd001011QM0mmm0 where Dddd=Qd, Mmmm=Qm, and Q=0.

  constexpr const char *Vmov = "vmov";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmov));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vmov));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vmov));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloat = false;

  const IValueT VmovOpcode = B25 | B21 | B8 | B4;

  if (Dd != Dm + 1)
    emitSIMDBase(VmovOpcode, Dd, Dm + 1, Dm + 1, UseQRegs, IsFloat);
  if (Dd + 1 != Dn + 1)
    emitSIMDBase(VmovOpcode, Dd + 1, Dn + 1, Dn + 1, UseQRegs, IsFloat);
}

void AssemblerARM32::vmovlhq(const Operand *OpQd, const Operand *OpQn,
                             const Operand *OpQm) {
  // Pseudo-instruction to copy the first source operand and insert the lower
  // half of the second operand into the high half of the destination.

  // VMOV (register) - ARMv7-A/R section A8.6.327, encoding A1:
  //   VMOV<c> <Dd>, <Dm>
  //
  // 111100111D110000ddd001011QM0mmm0 where Dddd=Qd, Mmmm=Qm, and Q=0.

  constexpr const char *Vmov = "vmov";
  const IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Qd", Vmov));
  const IValueT Dn = mapQRegToDReg(encodeQRegister(OpQn, "Qn", Vmov));
  const IValueT Dm = mapQRegToDReg(encodeQRegister(OpQm, "Qm", Vmov));

  constexpr bool UseQRegs = false;
  constexpr bool IsFloat = false;

  const IValueT VmovOpcode = B25 | B21 | B8 | B4;

  if (Dd + 1 != Dm)
    emitSIMDBase(VmovOpcode, Dd + 1, Dm, Dm, UseQRegs, IsFloat);
  if (Dd != Dn)
    emitSIMDBase(VmovOpcode, Dd, Dn, Dn, UseQRegs, IsFloat);
}

void AssemblerARM32::vnegqs(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm) {
  // VNEG - ARM section A8.8.355, encoding A1:
  //   vneg.<dt> <Qd>, <Qm>
  //
  // 111111111D11ss01dddd0F111QM0mmmm where Dddd=Qd, and Mmmm=Qm, and:
  //     * dt=s8  -> 00=ss, 0=F
  //     * dt=s16 -> 01=ss, 0=F
  //     * dt=s32 -> 10=ss, 0=F
  //     * dt=s32 -> 10=ss, 1=F
  constexpr const char *Vneg = "vneg";
  constexpr IValueT VnegOpcode = B24 | B23 | B21 | B20 | B16 | B9 | B8 | B7;
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vneg);
  constexpr IValueT Qn = 0;
  const IValueT Qm = encodeQRegister(OpQm, "Qm", Vneg);
  constexpr bool UseQRegs = true;
  constexpr IValueT ElmtShift = 18;
  const IValueT ElmtSize = encodeElmtType(ElmtTy);
  assert(Utils::IsUint(2, ElmtSize));
  emitSIMDBase(VnegOpcode | (ElmtSize << ElmtShift), mapQRegToDReg(Qd),
               mapQRegToDReg(Qn), mapQRegToDReg(Qm), UseQRegs,
               isFloatingType(ElmtTy));
}

void AssemblerARM32::vorrq(const Operand *OpQd, const Operand *OpQm,
                           const Operand *OpQn) {
  // VORR (register) - ARM section A8.8.360, encoding A1:
  //   vorr <Qd>, <Qn>, <Qm>
  //
  // 111100100D10nnn0ddd00001N1M1mmm0 where Dddd=OpQd, Nnnn=OpQm, and Mmmm=OpQm.
  constexpr const char *Vorrq = "vorrq";
  constexpr IValueT VorrqOpcode = B21 | B8 | B4;
  constexpr Type ElmtTy = IceType_i8;
  emitSIMDqqq(VorrqOpcode, ElmtTy, OpQd, OpQm, OpQn, Vorrq);
}

void AssemblerARM32::vstrd(const Operand *OpDd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // VSTR - ARM section A8.8.413, encoding A1:
  //   vstr<c> <Dd>, [<Rn>{, #+/-<Imm>}]
  //
  // cccc1101UD00nnnndddd1011iiiiiiii where cccc=Cond, nnnn=Rn, Ddddd=Rd,
  // iiiiiiii=abs(Imm >> 2), and U=1 if Imm>=0.
  constexpr const char *Vstrd = "vstrd";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vstrd);
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  IValueT AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding = B27 | B26 | B24 | B11 | B9 | B8 |
                     (encodeCondition(Cond) << kConditionShift) |
                     (getYInRegYXXXX(Dd) << 22) |
                     (getXXXXInRegYXXXX(Dd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::vstrq(const Operand *OpQd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // This is a pseudo-instruction which stores 64-bit data into a quadword
  // vector register. It is implemented by storing into the lower doubleword.

  // VSTR - ARM section A8.8.413, encoding A1:
  //   vstr<c> <Dd>, [<Rn>{, #+/-<Imm>}]
  //
  // cccc1101UD00nnnndddd1011iiiiiiii where cccc=Cond, nnnn=Rn, Ddddd=Rd,
  // iiiiiiii=abs(Imm >> 2), and U=1 if Imm>=0.
  constexpr const char *Vstrd = "vstrd";
  IValueT Dd = mapQRegToDReg(encodeQRegister(OpQd, "Dd", Vstrd));
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  IValueT AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding = B27 | B26 | B24 | B11 | B9 | B8 |
                     (encodeCondition(Cond) << kConditionShift) |
                     (getYInRegYXXXX(Dd) << 22) |
                     (getXXXXInRegYXXXX(Dd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::vstrs(const Operand *OpSd, const Operand *OpAddress,
                           CondARM32::Cond Cond, const TargetInfo &TInfo) {
  // VSTR - ARM section A8.8.413, encoding A2:
  //   vstr<c> <Sd>, [<Rn>{, #+/-<imm>]]
  //
  // cccc1101UD01nnnndddd1010iiiiiiii where cccc=Cond, nnnn=Rn, ddddD=Sd,
  // iiiiiiii=abs(Opcode), and U=1 if Opcode >= 0;
  constexpr const char *Vstrs = "vstrs";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vstrs);
  assert(CondARM32::isDefined(Cond));
  IValueT Address;
  IValueT AddressEncoding =
      encodeAddress(OpAddress, Address, TInfo, RotatedImm8Div4Address);
  (void)AddressEncoding;
  assert(AddressEncoding == EncodedAsImmRegOffset);
  IValueT Encoding =
      B27 | B26 | B24 | B11 | B9 | (encodeCondition(Cond) << kConditionShift) |
      (getYInRegXXXXY(Sd) << 22) | (getXXXXInRegXXXXY(Sd) << 12) | Address;
  emitInst(Encoding);
}

void AssemblerARM32::vst1qr(size_t ElmtSize, const Operand *OpQd,
                            const Operand *OpAddress, const TargetInfo &TInfo) {
  // VST1 (multiple single elements) - ARM section A8.8.404, encoding A1:
  //   vst1.<size> <Qd>, [<Rn>]
  //
  // 111101000D00nnnnddd0ttttssaammmm where tttt=DRegListSize2, Dddd=Qd,
  // nnnn=Rn, aa=0 (use default alignment), size=ElmtSize, and ss is the
  // encoding of ElmtSize.
  constexpr const char *Vst1qr = "vst1qr";
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vst1qr);
  const IValueT Dd = mapQRegToDReg(Qd);
  IValueT Address;
  if (encodeAddress(OpAddress, Address, TInfo, NoImmOffsetAddress) !=
      EncodedAsImmRegOffset)
    llvm::report_fatal_error(std::string(Vst1qr) + ": malform memory address");
  const IValueT Rn = mask(Address, kRnShift, 4);
  constexpr IValueT Rm = RegARM32::Reg_pc;
  constexpr IValueT Opcode = B26;
  constexpr IValueT Align = 0; // use default alignment.
  emitVMem1Op(Opcode, Dd, Rn, Rm, DRegListSize2, ElmtSize, Align, Vst1qr);
}

void AssemblerARM32::vst1(size_t ElmtSize, const Operand *OpQd,
                          const Operand *OpAddress, const TargetInfo &TInfo) {

  // This is a pseudo-instruction for storing a single element of a quadword
  // vector. For 64-bit the lower doubleword vector is stored.

  if (ElmtSize == 64) {
    return vstrq(OpQd, OpAddress, Ice::CondARM32::AL, TInfo);
  }

  // VST1 (single element from one lane) - ARMv7-A/R section A8.6.392, encoding
  // A1:
  //   VST1<c>.<size> <list>, [<Rn>{@<align>}], <Rm>
  //
  // 111101001D00nnnnddd0ss00aaaammmm where Dddd=Qd, nnnn=Rn,
  // aaaa=0 (use default alignment), size=ElmtSize, and ss is the
  // encoding of ElmtSize.
  constexpr const char *Vst1qr = "vst1qr";
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vst1qr);
  const IValueT Dd = mapQRegToDReg(Qd);
  IValueT Address;
  if (encodeAddress(OpAddress, Address, TInfo, NoImmOffsetAddress) !=
      EncodedAsImmRegOffset)
    llvm::report_fatal_error(std::string(Vst1qr) + ": malform memory address");
  const IValueT Rn = mask(Address, kRnShift, 4);
  constexpr IValueT Rm = RegARM32::Reg_pc;
  constexpr IValueT Opcode = B26 | B23;
  constexpr IValueT Align = 0; // use default alignment.
  emitVMem1Op(Opcode, Dd, Rn, Rm, ElmtSize, Align, Vst1qr);
}

void AssemblerARM32::vsubs(const Operand *OpSd, const Operand *OpSn,
                           const Operand *OpSm, CondARM32::Cond Cond) {
  // VSUB (floating-point) - ARM section A8.8.415, encoding A2:
  //   vsub<c>.f32 <Sd>, <Sn>, <Sm>
  //
  // cccc11100D11nnnndddd101sN1M0mmmm where cccc=Cond, s=0, ddddD=Rd, nnnnN=Rn,
  // and mmmmM=Rm.
  constexpr const char *Vsubs = "vsubs";
  constexpr IValueT VsubsOpcode = B21 | B20 | B6;
  emitVFPsss(Cond, VsubsOpcode, OpSd, OpSn, OpSm, Vsubs);
}

void AssemblerARM32::vsubd(const Operand *OpDd, const Operand *OpDn,
                           const Operand *OpDm, CondARM32::Cond Cond) {
  // VSUB (floating-point) - ARM section A8.8.415, encoding A2:
  //   vsub<c>.f64 <Dd>, <Dn>, <Dm>
  //
  // cccc11100D11nnnndddd101sN1M0mmmm where cccc=Cond, s=1, Ddddd=Rd, Nnnnn=Rn,
  // and Mmmmm=Rm.
  constexpr const char *Vsubd = "vsubd";
  constexpr IValueT VsubdOpcode = B21 | B20 | B6;
  emitVFPddd(Cond, VsubdOpcode, OpDd, OpDn, OpDm, Vsubd);
}

void AssemblerARM32::vqaddqi(Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // VQADD (integer) - ARM section A8.6.369, encoding A1:
  //   vqadd<c><q>.s<size> {<Qd>,} <Qn>, <Qm>
  //
  // 111100100Dssnnn0ddd00000N1M1mmm0 where Dddd=OpQd, Nnnn=OpQn, Mmmm=OpQm,
  // size is 8, 16, 32, or 64.
  assert(isScalarIntegerType(ElmtTy) &&
         "vqaddqi expects vector with integer element type");
  constexpr const char *Vqaddqi = "vqaddqi";
  constexpr IValueT VqaddqiOpcode = B4;
  emitSIMDqqq(VqaddqiOpcode, ElmtTy, OpQd, OpQm, OpQn, Vqaddqi);
}

void AssemblerARM32::vqaddqu(Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // VQADD (integer) - ARM section A8.6.369, encoding A1:
  //   vqadd<c><q>.s<size> {<Qd>,} <Qn>, <Qm>
  //
  // 111100110Dssnnn0ddd00000N1M1mmm0 where Dddd=OpQd, Nnnn=OpQn, Mmmm=OpQm,
  // size is 8, 16, 32, or 64.
  assert(isScalarIntegerType(ElmtTy) &&
         "vqaddqu expects vector with integer element type");
  constexpr const char *Vqaddqu = "vqaddqu";
  constexpr IValueT VqaddquOpcode = B24 | B4;
  emitSIMDqqq(VqaddquOpcode, ElmtTy, OpQd, OpQm, OpQn, Vqaddqu);
}

void AssemblerARM32::vqsubqi(Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // VQSUB (integer) - ARM section A8.6.369, encoding A1:
  //   vqsub<c><q>.s<size> {<Qd>,} <Qn>, <Qm>
  //
  // 111100100Dssnnn0ddd00010N1M1mmm0 where Dddd=OpQd, Nnnn=OpQn, Mmmm=OpQm,
  // size is 8, 16, 32, or 64.
  assert(isScalarIntegerType(ElmtTy) &&
         "vqsubqi expects vector with integer element type");
  constexpr const char *Vqsubqi = "vqsubqi";
  constexpr IValueT VqsubqiOpcode = B9 | B4;
  emitSIMDqqq(VqsubqiOpcode, ElmtTy, OpQd, OpQm, OpQn, Vqsubqi);
}

void AssemblerARM32::vqsubqu(Type ElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn) {
  // VQSUB (integer) - ARM section A8.6.369, encoding A1:
  //   vqsub<c><q>.s<size> {<Qd>,} <Qn>, <Qm>
  //
  // 111100110Dssnnn0ddd00010N1M1mmm0 where Dddd=OpQd, Nnnn=OpQn, Mmmm=OpQm,
  // size is 8, 16, 32, or 64.
  assert(isScalarIntegerType(ElmtTy) &&
         "vqsubqu expects vector with integer element type");
  constexpr const char *Vqsubqu = "vqsubqu";
  constexpr IValueT VqsubquOpcode = B24 | B9 | B4;
  emitSIMDqqq(VqsubquOpcode, ElmtTy, OpQd, OpQm, OpQn, Vqsubqu);
}

void AssemblerARM32::vsubqi(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // VSUB (integer) - ARM section A8.8.414, encoding A1:
  //   vsub.<dt> <Qd>, <Qn>, <Qm>
  //
  // 111100110Dssnnn0ddd01000N1M0mmm0 where Dddd=OpQd, Nnnn=OpQm, Mmmm=OpQm,
  // and dt in [i8, i16, i32, i64] where ss is the index.
  assert(isScalarIntegerType(ElmtTy) &&
         "vsubqi expects vector with integer element type");
  constexpr const char *Vsubqi = "vsubqi";
  constexpr IValueT VsubqiOpcode = B24 | B11;
  emitSIMDqqq(VsubqiOpcode, ElmtTy, OpQd, OpQm, OpQn, Vsubqi);
}

void AssemblerARM32::vqmovn2(Type DestElmtTy, const Operand *OpQd,
                             const Operand *OpQm, const Operand *OpQn,
                             bool Unsigned, bool Saturating) {
  // Pseudo-instruction for packing two quadword vectors into one quadword
  // vector, narrowing each element using saturation or truncation.

  // VQMOVN - ARMv7-A/R section A8.6.361, encoding A1:
  //   V{Q}MOVN{U}N<c>.<type><size> <Dd>, <Qm>
  //
  // 111100111D11ss10dddd0010opM0mmm0 where Ddddd=OpQd, op = 10, Mmmm=OpQm,
  // ss is 00 (16-bit), 01 (32-bit), or 10 (64-bit).

  assert(DestElmtTy != IceType_i64 &&
         "vmovn doesn't allow i64 destination vector elements!");

  constexpr const char *Vqmovn = "vqmovn";
  constexpr bool UseQRegs = false;
  constexpr bool IsFloatTy = false;
  const IValueT Qd = encodeQRegister(OpQd, "Qd", Vqmovn);
  const IValueT Qm = encodeQRegister(OpQm, "Qm", Vqmovn);
  const IValueT Qn = encodeQRegister(OpQn, "Qn", Vqmovn);
  const IValueT Dd = mapQRegToDReg(Qd);
  const IValueT Dm = mapQRegToDReg(Qm);
  const IValueT Dn = mapQRegToDReg(Qn);

  IValueT VqmovnOpcode = B25 | B24 | B23 | B21 | B20 | B17 | B9 |
                         (Saturating ? (Unsigned ? B6 : B7) : 0);

  constexpr IValueT ElmtShift = 18;
  VqmovnOpcode |= (encodeElmtType(DestElmtTy) << ElmtShift);

  if (Qm != Qd) {
    // Narrow second source operand to upper half of destination.
    emitSIMDBase(VqmovnOpcode, Dd + 1, 0, Dn, UseQRegs, IsFloatTy);
    // Narrow first source operand to lower half of destination.
    emitSIMDBase(VqmovnOpcode, Dd + 0, 0, Dm, UseQRegs, IsFloatTy);
  } else if (Qn != Qd) {
    // Narrow first source operand to lower half of destination.
    emitSIMDBase(VqmovnOpcode, Dd + 0, 0, Dm, UseQRegs, IsFloatTy);
    // Narrow second source operand to upper half of destination.
    emitSIMDBase(VqmovnOpcode, Dd + 1, 0, Dn, UseQRegs, IsFloatTy);
  } else {
    // Narrow first source operand to lower half of destination.
    emitSIMDBase(VqmovnOpcode, Dd, 0, Dm, UseQRegs, IsFloatTy);

    // VMOV Dd, Dm
    // 111100100D10mmmmdddd0001MQM1mmmm
    const IValueT VmovOpcode = B25 | B21 | B8 | B4;

    emitSIMDBase(VmovOpcode, Dd + 1, Dd, Dd, UseQRegs, IsFloatTy);
  }
}

void AssemblerARM32::vsubqf(const Operand *OpQd, const Operand *OpQn,
                            const Operand *OpQm) {
  // VSUB (floating-point) - ARM section A8.8.415, Encoding A1:
  //   vsub.f32 <Qd>, <Qn>, <Qm>
  //
  // 111100100D10nnn0ddd01101N1M0mmm0 where Dddd=Qd, Nnnn=Qn, and Mmmm=Qm.
  assert(OpQd->getType() == IceType_v4f32 && "vsubqf expects type <4 x float>");
  constexpr const char *Vsubqf = "vsubqf";
  constexpr IValueT VsubqfOpcode = B21 | B11 | B8;
  emitSIMDqqq(VsubqfOpcode, IceType_f32, OpQd, OpQn, OpQm, Vsubqf);
}

void AssemblerARM32::emitVStackOp(CondARM32::Cond Cond, IValueT Opcode,
                                  const Variable *OpBaseReg,
                                  SizeT NumConsecRegs) {
  const IValueT BaseReg = getEncodedSRegNum(OpBaseReg);
  const IValueT DLastBit = mask(BaseReg, 0, 1); // Last bit of base register.
  const IValueT Rd = mask(BaseReg, 1, 4);       // Top 4 bits of base register.
  assert(0 < NumConsecRegs);
  (void)VpushVpopMaxConsecRegs;
  assert(NumConsecRegs <= VpushVpopMaxConsecRegs);
  assert((BaseReg + NumConsecRegs) <= RegARM32::getNumSRegs());
  assert(CondARM32::isDefined(Cond));
  const IValueT Encoding = Opcode | (Cond << kConditionShift) | DLastBit |
                           (Rd << kRdShift) | NumConsecRegs;
  emitInst(Encoding);
}

void AssemblerARM32::vpop(const Variable *OpBaseReg, SizeT NumConsecRegs,
                          CondARM32::Cond Cond) {
  // Note: Current implementation assumes that OpBaseReg is defined using S
  // registers. It doesn't implement the D register form.
  //
  // VPOP - ARM section A8.8.367, encoding A2:
  //  vpop<c> <RegList>
  //
  // cccc11001D111101dddd1010iiiiiiii where cccc=Cond, ddddD=BaseReg, and
  // iiiiiiii=NumConsecRegs.
  constexpr IValueT VpopOpcode =
      B27 | B26 | B23 | B21 | B20 | B19 | B18 | B16 | B11 | B9;
  emitVStackOp(Cond, VpopOpcode, OpBaseReg, NumConsecRegs);
}

void AssemblerARM32::vpush(const Variable *OpBaseReg, SizeT NumConsecRegs,
                           CondARM32::Cond Cond) {
  // Note: Current implementation assumes that OpBaseReg is defined using S
  // registers. It doesn't implement the D register form.
  //
  // VPUSH - ARM section A8.8.368, encoding A2:
  //   vpush<c> <RegList>
  //
  // cccc11010D101101dddd1010iiiiiiii where cccc=Cond, ddddD=BaseReg, and
  // iiiiiiii=NumConsecRegs.
  constexpr IValueT VpushOpcode =
      B27 | B26 | B24 | B21 | B19 | B18 | B16 | B11 | B9;
  emitVStackOp(Cond, VpushOpcode, OpBaseReg, NumConsecRegs);
}

void AssemblerARM32::vshlqi(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // VSHL - ARM section A8.8.396, encoding A1:
  //   vshl Qd, Qm, Qn
  //
  // 1111001U0Dssnnnndddd0100NQM0mmmm where Ddddd=Qd, Mmmmm=Qm, Nnnnn=Qn, 0=U,
  // 1=Q
  assert(isScalarIntegerType(ElmtTy) &&
         "vshl expects vector with integer element type");
  constexpr const char *Vshl = "vshl";
  constexpr IValueT VshlOpcode = B10 | B6;
  emitSIMDqqq(VshlOpcode, ElmtTy, OpQd, OpQn, OpQm, Vshl);
}

void AssemblerARM32::vshlqc(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm,
                            const ConstantInteger32 *Imm6) {
  // VSHL - ARM section A8.8.395, encoding A1:
  //   vshl Qd, Qm, #Imm
  //
  // 1111001U1Diiiiiidddd0101LQM1mmmm where Ddddd=Qd, Mmmmm=Qm, iiiiii=Imm6,
  // 0=U, 1=Q, 0=L.
  assert(isScalarIntegerType(ElmtTy) &&
         "vshl expects vector with integer element type");
  constexpr const char *Vshl = "vshl";
  constexpr IValueT VshlOpcode = B23 | B10 | B8 | B4;
  emitSIMDShiftqqc(VshlOpcode, OpQd, OpQm,
                   encodeSIMDShiftImm6(ST_Vshl, ElmtTy, Imm6), Vshl);
}

void AssemblerARM32::vshrqc(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const ConstantInteger32 *Imm6,
                            InstARM32::FPSign Sign) {
  // VSHR - ARM section A8.8.398, encoding A1:
  //   vshr Qd, Qm, #Imm
  //
  // 1111001U1Diiiiiidddd0101LQM1mmmm where Ddddd=Qd, Mmmmm=Qm, iiiiii=Imm6,
  // U=Unsigned, Q=1, L=0.
  assert(isScalarIntegerType(ElmtTy) &&
         "vshr expects vector with integer element type");
  constexpr const char *Vshr = "vshr";
  const IValueT VshrOpcode =
      (Sign == InstARM32::FS_Unsigned ? B24 : 0) | B23 | B4;
  emitSIMDShiftqqc(VshrOpcode, OpQd, OpQm,
                   encodeSIMDShiftImm6(ST_Vshr, ElmtTy, Imm6), Vshr);
}

void AssemblerARM32::vshlqu(Type ElmtTy, const Operand *OpQd,
                            const Operand *OpQm, const Operand *OpQn) {
  // VSHL - ARM section A8.8.396, encoding A1:
  //   vshl Qd, Qm, Qn
  //
  // 1111001U0Dssnnnndddd0100NQM0mmmm where Ddddd=Qd, Mmmmm=Qm, Nnnnn=Qn, 1=U,
  // 1=Q
  assert(isScalarIntegerType(ElmtTy) &&
         "vshl expects vector with integer element type");
  constexpr const char *Vshl = "vshl";
  constexpr IValueT VshlOpcode = B24 | B10 | B6;
  emitSIMDqqq(VshlOpcode, ElmtTy, OpQd, OpQn, OpQm, Vshl);
}

void AssemblerARM32::vsqrtd(const Operand *OpDd, const Operand *OpDm,
                            CondARM32::Cond Cond) {
  // VSQRT - ARM section A8.8.401, encoding A1:
  //   vsqrt<c>.f64 <Dd>, <Dm>
  //
  // cccc11101D110001dddd101111M0mmmm where cccc=Cond, Ddddd=Sd, and Mmmmm=Sm.
  constexpr const char *Vsqrtd = "vsqrtd";
  IValueT Dd = encodeDRegister(OpDd, "Dd", Vsqrtd);
  IValueT Dm = encodeDRegister(OpDm, "Dm", Vsqrtd);
  constexpr IValueT VsqrtdOpcode = B23 | B21 | B20 | B16 | B7 | B6;
  constexpr IValueT D0 = 0;
  emitVFPddd(Cond, VsqrtdOpcode, Dd, D0, Dm);
}

void AssemblerARM32::vsqrts(const Operand *OpSd, const Operand *OpSm,
                            CondARM32::Cond Cond) {
  // VSQRT - ARM section A8.8.401, encoding A1:
  //   vsqrt<c>.f32 <Sd>, <Sm>
  //
  // cccc11101D110001dddd101011M0mmmm where cccc=Cond, ddddD=Sd, and mmmmM=Sm.
  constexpr const char *Vsqrts = "vsqrts";
  IValueT Sd = encodeSRegister(OpSd, "Sd", Vsqrts);
  IValueT Sm = encodeSRegister(OpSm, "Sm", Vsqrts);
  constexpr IValueT VsqrtsOpcode = B23 | B21 | B20 | B16 | B7 | B6;
  constexpr IValueT S0 = 0;
  emitVFPsss(Cond, VsqrtsOpcode, Sd, S0, Sm);
}

} // end of namespace ARM32
} // end of namespace Ice
