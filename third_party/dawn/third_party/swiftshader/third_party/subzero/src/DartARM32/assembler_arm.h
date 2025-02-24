// Copyright (c) 2013, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
//
// This is forked from Dart revision df52deea9f25690eb8b66c5995da92b70f7ac1fe
// Please update the (git) revision if we merge changes from Dart.
// https://code.google.com/p/dart/wiki/GettingTheSource

#ifndef VM_ASSEMBLER_ARM_H_
#define VM_ASSEMBLER_ARM_H_

#ifndef VM_ASSEMBLER_H_
#error Do not include assembler_arm.h directly; use assembler.h instead.
#endif

#include "platform/assert.h"
#include "platform/utils.h"
#include "vm/constants_arm.h"
#include "vm/cpu.h"
#include "vm/hash_map.h"
#include "vm/object.h"
#include "vm/simulator.h"

namespace dart {

// Forward declarations.
class RuntimeEntry;
class StubEntry;

#if 0
// Moved to ARM32::AssemblerARM32 as needed
// Instruction encoding bits.
enum {
  H   = 1 << 5,   // halfword (or byte)
  L   = 1 << 20,  // load (or store)
  S   = 1 << 20,  // set condition code (or leave unchanged)
  W   = 1 << 21,  // writeback base register (or leave unchanged)
  A   = 1 << 21,  // accumulate in multiply instruction (or not)
  B   = 1 << 22,  // unsigned byte (or word)
  D   = 1 << 22,  // high/lo bit of start of s/d register range
  N   = 1 << 22,  // long (or short)
  U   = 1 << 23,  // positive (or negative) offset/index
  P   = 1 << 24,  // offset/pre-indexed addressing (or post-indexed addressing)
  I   = 1 << 25,  // immediate shifter operand (or not)

  B0 = 1,
  B1 = 1 << 1,
  B2 = 1 << 2,
  B3 = 1 << 3,
  B4 = 1 << 4,
  B5 = 1 << 5,
  B6 = 1 << 6,
  B7 = 1 << 7,
  B8 = 1 << 8,
  B9 = 1 << 9,
  B10 = 1 << 10,
  B11 = 1 << 11,
  B12 = 1 << 12,
  B16 = 1 << 16,
  B17 = 1 << 17,
  B18 = 1 << 18,
  B19 = 1 << 19,
  B20 = 1 << 20,
  B21 = 1 << 21,
  B22 = 1 << 22,
  B23 = 1 << 23,
  B24 = 1 << 24,
  B25 = 1 << 25,
  B26 = 1 << 26,
  B27 = 1 << 27,
};
#endif

class Label : public ValueObject {
public:
  Label() : position_(0) {}

  ~Label() {
    // Assert if label is being destroyed with unresolved branches pending.
    ASSERT(!IsLinked());
  }

  // Returns the position for bound and linked labels. Cannot be used
  // for unused labels.
  intptr_t Position() const {
    ASSERT(!IsUnused());
    return IsBound() ? -position_ - kWordSize : position_ - kWordSize;
  }

  bool IsBound() const { return position_ < 0; }
  bool IsUnused() const { return position_ == 0; }
  bool IsLinked() const { return position_ > 0; }

private:
  intptr_t position_;

  void Reinitialize() { position_ = 0; }

  void BindTo(intptr_t position) {
    ASSERT(!IsBound());
    position_ = -position - kWordSize;
    ASSERT(IsBound());
  }

  void LinkTo(intptr_t position) {
    ASSERT(!IsBound());
    position_ = position + kWordSize;
    ASSERT(IsLinked());
  }

  friend class Assembler;
  DISALLOW_COPY_AND_ASSIGN(Label);
};

// Encodes Addressing Mode 1 - Data-processing operands.
class Operand : public ValueObject {
public:
  // Data-processing operands - Uninitialized.
  Operand() : type_(-1), encoding_(-1) {}

  // Data-processing operands - Copy constructor.
  Operand(const Operand &other)
      : ValueObject(), type_(other.type_), encoding_(other.encoding_) {}

  // Data-processing operands - Assignment operator.
  Operand &operator=(const Operand &other) {
    type_ = other.type_;
    encoding_ = other.encoding_;
    return *this;
  }

#if 0
  // Moved to encodeRotatedImm8() in IceAssemblerARM32.cpp
  // Data-processing operands - Immediate.
  explicit Operand(uint32_t immediate) {
    ASSERT(immediate < (1 << kImmed8Bits));
    type_ = 1;
    encoding_ = immediate;
  }

  // Moved to decodeOperand() and encodeRotatedImm8() in IceAssemblerARM32.cpp
  // Data-processing operands - Rotated immediate.
  Operand(uint32_t rotate, uint32_t immed8) {
    ASSERT((rotate < (1 << kRotateBits)) && (immed8 < (1 << kImmed8Bits)));
    type_ = 1;
    encoding_ = (rotate << kRotateShift) | (immed8 << kImmed8Shift);
  }

  // Moved to decodeOperand() in IceAssemblerARM32.cpp
  // Data-processing operands - Register.
  explicit Operand(Register rm) {
    type_ = 0;
    encoding_ = static_cast<uint32_t>(rm);
  }

  // Moved to encodeShiftRotateImm5() in IceAssemblerARM32.cpp
  // Data-processing operands - Logical shift/rotate by immediate.
  Operand(Register rm, Shift shift, uint32_t shift_imm) {
    ASSERT(shift_imm < (1 << kShiftImmBits));
    type_ = 0;
    encoding_ = shift_imm << kShiftImmShift |
                static_cast<uint32_t>(shift) << kShiftShift |
                static_cast<uint32_t>(rm);
  }

  // Moved to encodeShiftRotateReg() in IceAssemblerARM32.cpp
  // Data-processing operands - Logical shift/rotate by register.
  Operand(Register rm, Shift shift, Register rs) {
    type_ = 0;
    encoding_ = static_cast<uint32_t>(rs) << kShiftRegisterShift |
                static_cast<uint32_t>(shift) << kShiftShift | (1 << 4) |
                static_cast<uint32_t>(rm);
  }

  // Already defined as ARM32::OperandARM32FlexImm::canHoldImm().
  static bool CanHold(uint32_t immediate, Operand* o) {
    // Avoid the more expensive test for frequent small immediate values.
    if (immediate < (1 << kImmed8Bits)) {
      o->type_ = 1;
      o->encoding_ = (0 << kRotateShift) | (immediate << kImmed8Shift);
      return true;
    }
    // Note that immediate must be unsigned for the test to work correctly.
    for (int rot = 0; rot < 16; rot++) {
      uint32_t imm8 = (immediate << 2*rot) | (immediate >> (32 - 2*rot));
      if (imm8 < (1 << kImmed8Bits)) {
        o->type_ = 1;
        o->encoding_ = (rot << kRotateShift) | (imm8 << kImmed8Shift);
        return true;
      }
    }
    return false;
  }
#endif

private:
  bool is_valid() const { return (type_ == 0) || (type_ == 1); }

  uint32_t type() const {
    ASSERT(is_valid());
    return type_;
  }

  uint32_t encoding() const {
    ASSERT(is_valid());
    return encoding_;
  }

  uint32_t type_; // Encodes the type field (bits 27-25) in the instruction.
  uint32_t encoding_;

  friend class Assembler;
  friend class Address;
};

enum OperandSize {
  kByte,
  kUnsignedByte,
  kHalfword,
  kUnsignedHalfword,
  kWord,
  kUnsignedWord,
  kWordPair,
  kSWord,
  kDWord,
  kRegList,
};

// Load/store multiple addressing mode.
enum BlockAddressMode {
  // clang-format off
  // bit encoding P U W
  DA           = (0|0|0) << 21,  // decrement after
  IA           = (0|4|0) << 21,  // increment after
  DB           = (8|0|0) << 21,  // decrement before
  IB           = (8|4|0) << 21,  // increment before
  DA_W         = (0|0|1) << 21,  // decrement after with writeback to base
  IA_W         = (0|4|1) << 21,  // increment after with writeback to base
  DB_W         = (8|0|1) << 21,  // decrement before with writeback to base
  IB_W         = (8|4|1) << 21   // increment before with writeback to base
  // clang-format on
};

class Address : public ValueObject {
public:
  enum OffsetKind {
    Immediate,
    IndexRegister,
    ScaledIndexRegister,
  };

  // Memory operand addressing mode
  enum Mode {
    // clang-format off
    kModeMask    = (8|4|1) << 21,
    // bit encoding P U W
    Offset       = (8|4|0) << 21,  // offset (w/o writeback to base)
    PreIndex     = (8|4|1) << 21,  // pre-indexed addressing with writeback
    PostIndex    = (0|4|0) << 21,  // post-indexed addressing with writeback
    NegOffset    = (8|0|0) << 21,  // negative offset (w/o writeback to base)
    NegPreIndex  = (8|0|1) << 21,  // negative pre-indexed with writeback
    NegPostIndex = (0|0|0) << 21   // negative post-indexed with writeback
    // clang-format on
  };

  Address(const Address &other)
      : ValueObject(), encoding_(other.encoding_), kind_(other.kind_) {}

  Address &operator=(const Address &other) {
    encoding_ = other.encoding_;
    kind_ = other.kind_;
    return *this;
  }

  bool Equals(const Address &other) const {
    return (encoding_ == other.encoding_) && (kind_ == other.kind_);
  }

#if 0
  // Moved to decodeImmRegOffset() in IceAssemblerARM32.cpp.
  // Used to model stack offsets.
  explicit Address(Register rn, int32_t offset = 0, Mode am = Offset) {
    ASSERT(Utils::IsAbsoluteUint(12, offset));
    kind_ = Immediate;
    if (offset < 0) {
      encoding_ = (am ^ (1 << kUShift)) | -offset;  // Flip U to adjust sign.
    } else {
      encoding_ = am | offset;
    }
    encoding_ |= static_cast<uint32_t>(rn) << kRnShift;
  }
#endif

  // There is no register offset mode unless Mode is Offset, in which case the
  // shifted register case below should be used.
  Address(Register rn, Register r, Mode am);

  Address(Register rn, Register rm, Shift shift = LSL, uint32_t shift_imm = 0,
          Mode am = Offset) {
    Operand o(rm, shift, shift_imm);

    if ((shift == LSL) && (shift_imm == 0)) {
      kind_ = IndexRegister;
    } else {
      kind_ = ScaledIndexRegister;
    }
    encoding_ = o.encoding() | am | (static_cast<uint32_t>(rn) << kRnShift);
  }

  // There is no shifted register mode with a register shift.
  Address(Register rn, Register rm, Shift shift, Register r, Mode am = Offset);

  static OperandSize OperandSizeFor(intptr_t cid);

  static bool CanHoldLoadOffset(OperandSize size, int32_t offset,
                                int32_t *offset_mask);
  static bool CanHoldStoreOffset(OperandSize size, int32_t offset,
                                 int32_t *offset_mask);
  static bool CanHoldImmediateOffset(bool is_load, intptr_t cid,
                                     int64_t offset);

private:
  Register rn() const {
    return Instr::At(reinterpret_cast<uword>(&encoding_))->RnField();
  }

  Register rm() const {
    return ((kind() == IndexRegister) || (kind() == ScaledIndexRegister))
               ? Instr::At(reinterpret_cast<uword>(&encoding_))->RmField()
               : kNoRegister;
  }

  Mode mode() const { return static_cast<Mode>(encoding() & kModeMask); }

  uint32_t encoding() const { return encoding_; }

#if 0
// Moved to encodeImmRegOffsetEnc3 in IceAssemblerARM32.cpp
  // Encoding for addressing mode 3.
  uint32_t encoding3() const;
#endif

  // Encoding for vfp load/store addressing.
  uint32_t vencoding() const;

  OffsetKind kind() const { return kind_; }

  uint32_t encoding_;

  OffsetKind kind_;

  friend class Assembler;
};

class FieldAddress : public Address {
public:
  FieldAddress(Register base, int32_t disp)
      : Address(base, disp - kHeapObjectTag) {}

  // This addressing mode does not exist.
  FieldAddress(Register base, Register r);

  FieldAddress(const FieldAddress &other) : Address(other) {}

  FieldAddress &operator=(const FieldAddress &other) {
    Address::operator=(other);
    return *this;
  }
};

class Assembler : public ValueObject {
public:
  explicit Assembler(bool use_far_branches = false)
      : buffer_(), prologue_offset_(-1), use_far_branches_(use_far_branches),
        comments_(), constant_pool_allowed_(false) {}

  ~Assembler() {}

  void PopRegister(Register r) { Pop(r); }

  void Bind(Label *label);
  void Jump(Label *label) { b(label); }

  // Misc. functionality
  intptr_t CodeSize() const { return buffer_.Size(); }
  intptr_t prologue_offset() const { return prologue_offset_; }

  // Count the fixups that produce a pointer offset, without processing
  // the fixups.  On ARM there are no pointers in code.
  intptr_t CountPointerOffsets() const { return 0; }

  const ZoneGrowableArray<intptr_t> &GetPointerOffsets() const {
    ASSERT(buffer_.pointer_offsets().length() == 0); // No pointers in code.
    return buffer_.pointer_offsets();
  }

  ObjectPoolWrapper &object_pool_wrapper() { return object_pool_wrapper_; }

  RawObjectPool *MakeObjectPool() {
    return object_pool_wrapper_.MakeObjectPool();
  }

  bool use_far_branches() const {
    return FLAG_use_far_branches || use_far_branches_;
  }

#if defined(TESTING) || defined(DEBUG)
  // Used in unit tests and to ensure predictable verification code size in
  // FlowGraphCompiler::EmitEdgeCounter.
  void set_use_far_branches(bool b) { use_far_branches_ = b; }
#endif // TESTING || DEBUG

  void FinalizeInstructions(const MemoryRegion &region) {
    buffer_.FinalizeInstructions(region);
  }

  // Debugging and bringup support.
  void Stop(const char *message);
  void Unimplemented(const char *message);
  void Untested(const char *message);
  void Unreachable(const char *message);

  static void InitializeMemoryWithBreakpoints(uword data, intptr_t length);

  void Comment(const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
  static bool EmittingComments();

  const Code::Comments &GetCodeComments() const;

  static const char *RegisterName(Register reg);

  static const char *FpuRegisterName(FpuRegister reg);

#if 0
  // Moved to ARM32::AssemblerARM32::and_()
  // Data-processing instructions.
  void and_(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::eor()
  void eor(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemberARM32::sub()
  void sub(Register rd, Register rn, Operand o, Condition cond = AL);
  void subs(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemberARM32::rsb()
  void rsb(Register rd, Register rn, Operand o, Condition cond = AL);
  void rsbs(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::add()
  void add(Register rd, Register rn, Operand o, Condition cond = AL);

  void adds(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::adc()
  void adc(Register rd, Register rn, Operand o, Condition cond = AL);

  void adcs(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::sbc()
  void sbc(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::sbc()
  void sbcs(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::rsc()
  void rsc(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::tst();
  void tst(Register rn, Operand o, Condition cond = AL);
#endif

  void teq(Register rn, Operand o, Condition cond = AL);

#if 0
  // Moved to ARM32::AssemblerARM32::cmp()
  void cmp(Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::cmn()
  void cmn(Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::IceAssemblerARM32::orr().
  void orr(Register rd, Register rn, Operand o, Condition cond = AL);
  void orrs(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::IceAssemblerARM32::mov()
  void mov(Register rd, Operand o, Condition cond = AL);
  void movs(Register rd, Operand o, Condition cond = AL);

  // Moved to ARM32::IceAssemblerARM32::bic()
  void bic(Register rd, Register rn, Operand o, Condition cond = AL);
  void bics(Register rd, Register rn, Operand o, Condition cond = AL);

  // Moved to ARM32::IceAssemblerARM32::mvn()
  void mvn(Register rd, Operand o, Condition cond = AL);
  void mvns(Register rd, Operand o, Condition cond = AL);

  // Miscellaneous data-processing instructions.
  // Moved to ARM32::AssemblerARM32::clz()
  void clz(Register rd, Register rm, Condition cond = AL);

  // Multiply instructions.

  // Moved to ARM32::AssemblerARM32::mul()
  void mul(Register rd, Register rn, Register rm, Condition cond = AL);
  void muls(Register rd, Register rn, Register rm, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::mla()
  void mla(Register rd, Register rn, Register rm, Register ra,
           Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::mls()
  void mls(Register rd, Register rn, Register rm, Register ra,
           Condition cond = AL);
#endif

  void smull(Register rd_lo, Register rd_hi, Register rn, Register rm,
             Condition cond = AL);

#if 0
  // Moved to ARM32::AssemblerARM32::umull();
  void umull(Register rd_lo, Register rd_hi, Register rn, Register rm,
             Condition cond = AL);
#endif
  void smlal(Register rd_lo, Register rd_hi, Register rn, Register rm,
             Condition cond = AL);
  void umlal(Register rd_lo, Register rd_hi, Register rn, Register rm,
             Condition cond = AL);

  // Emulation of this instruction uses IP and the condition codes. Therefore,
  // none of the registers can be IP, and the instruction can only be used
  // unconditionally.
  void umaal(Register rd_lo, Register rd_hi, Register rn, Register rm);

  // Division instructions.
#if 0
  // Moved to ARM32::AssemblerARM32::sdiv()
  void sdiv(Register rd, Register rn, Register rm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::udiv()
  void udiv(Register rd, Register rn, Register rm, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::ldr()
  // Load/store instructions.
  void ldr(Register rd, Address ad, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::str()
  void str(Register rd, Address ad, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::ldr()
  void ldrb(Register rd, Address ad, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::str()
  void strb(Register rd, Address ad, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::ldr()
  void ldrh(Register rd, Address ad, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::str()
  void strh(Register rd, Address ad, Condition cond = AL);
#endif

  void ldrsb(Register rd, Address ad, Condition cond = AL);
  void ldrsh(Register rd, Address ad, Condition cond = AL);

  // ldrd and strd actually support the full range of addressing modes, but
  // we don't use them, and we need to split them up into two instructions for
  // ARMv5TE, so we only support the base + offset mode.
  void ldrd(Register rd, Register rn, int32_t offset, Condition cond = AL);
  void strd(Register rd, Register rn, int32_t offset, Condition cond = AL);

#if 0
  // Folded into ARM32::AssemblerARM32::popList(), since it is its only use (and
  // doesn't implement ARM LDM instructions).
  void ldm(BlockAddressMode am, Register base,
           RegList regs, Condition cond = AL);

  // Folded into ARM32::AssemblerARM32::pushList(), since it is its only use
  // (and doesn't implement ARM STM instruction).
  void stm(BlockAddressMode am, Register base,
           RegList regs, Condition cond = AL);

  // Moved to ARM::AssemblerARM32::ldrex();
  void ldrex(Register rd, Register rn, Condition cond = AL);
  // Moved to ARM::AssemblerARM32::strex();
  void strex(Register rd, Register rt, Register rn, Condition cond = AL);
#endif

  // Miscellaneous instructions.
  void clrex();

#if 0
  // Moved to ARM32::AssemblerARM32::nop().
  void nop(Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::bkpt()
  // Note that gdb sets breakpoints using the undefined instruction 0xe7f001f0.
  void bkpt(uint16_t imm16);

  static int32_t BkptEncoding(uint16_t imm16) {
    // bkpt requires that the cond field is AL.
    return (AL << kConditionShift) | B24 | B21 |
           ((imm16 >> 4) << 8) | B6 | B5 | B4 | (imm16 & 0xf);
  }

  // Not ported. PNaCl doesn't allow breakpoint instructions.
  static uword GetBreakInstructionFiller() {
    return BkptEncoding(0);
  }

  // Floating point instructions (VFPv3-D16 and VFPv3-D32 profiles).

  // Moved to ARM32::AssemblerARM32::vmovsr().
  void vmovsr(SRegister sn, Register rt, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovrs().
  void vmovrs(Register rt, SRegister sn, Condition cond = AL);
#endif
  void vmovsrr(SRegister sm, Register rt, Register rt2, Condition cond = AL);
  void vmovrrs(Register rt, Register rt2, SRegister sm, Condition cond = AL);
#if 0
  // Moved to ARM32::AssemblerARM32::vmovdrr().
  void vmovdrr(DRegister dm, Register rt, Register rt2, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovrrd().
  void vmovrrd(Register rt, Register rt2, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovqir().
  void vmovdr(DRegister dd, int i, Register rt, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovss().
  void vmovs(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovdd().
  void vmovd(DRegister dd, DRegister dm, Condition cond = AL);
#endif
  void vmovq(QRegister qd, QRegister qm);

#if 0
  // Returns false if the immediate cannot be encoded.
  // Moved to ARM32::AssemblerARM32::vmovs();
  bool vmovs(SRegister sd, float s_imm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmovs();
  bool vmovd(DRegister dd, double d_imm, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::vldrs()
  void vldrs(SRegister sd, Address ad, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vstrs()
  void vstrs(SRegister sd, Address ad, Condition cond = AL);
#endif
  // Moved to ARM32::AssemblerARM32::vldrd()
  void vldrd(DRegister dd, Address ad, Condition cond = AL);
#if 0
  // Moved to Arm32::AssemblerARM32::vstrd()
  void vstrd(DRegister dd, Address ad, Condition cond = AL);
#endif

  void vldms(BlockAddressMode am, Register base, SRegister first,
             SRegister last, Condition cond = AL);
  void vstms(BlockAddressMode am, Register base, SRegister first,
             SRegister last, Condition cond = AL);

  void vldmd(BlockAddressMode am, Register base, DRegister first,
             intptr_t count, Condition cond = AL);
  void vstmd(BlockAddressMode am, Register base, DRegister first,
             intptr_t count, Condition cond = AL);

#if 0
  // Moved to Arm32::AssemblerARM32::vadds()
  void vadds(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vaddd()
  void vaddd(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vaddqi().
  void vaddqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vaddqf().
  void vaddqs(QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vsubs()
  void vsubs(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vsubd()
  void vsubd(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vsubqi().
  void vsubqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vsubqf().
  void vsubqs(QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vmuls()
  void vmuls(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vmuld()
  void vmuld(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vmulqi().
  void vmulqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vmulqf().
  void vmulqs(QRegister qd, QRegister qn, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vshlqi().
  void vshlqi(OperandSize sz, QRegister qd, QRegister qm, QRegister qn);
  // Moved to ARM32::AssemblerARM32::vshlqu().
  void vshlqu(OperandSize sz, QRegister qd, QRegister qm, QRegister qn);
  // Moved to Arm32::AssemblerARM32::vmlas()
  void vmlas(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vmlad()
  void vmlad(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vmlss()
  void vmlss(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vmlsd()
  void vmlsd(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vdivs()
  void vdivs(SRegister sd, SRegister sn, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vdivd()
  void vdivd(DRegister dd, DRegister dn, DRegister dm, Condition cond = AL);
#endif
  void vminqs(QRegister qd, QRegister qn, QRegister qm);
  void vmaxqs(QRegister qd, QRegister qn, QRegister qm);
  void vrecpeqs(QRegister qd, QRegister qm);
  void vrecpsqs(QRegister qd, QRegister qn, QRegister qm);
  void vrsqrteqs(QRegister qd, QRegister qm);
  void vrsqrtsqs(QRegister qd, QRegister qn, QRegister qm);

#if 0
  // Moved to ARM32::AssemblerARM32::vorrq()
  void veorq(QRegister qd, QRegister qn, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vorrq()
  void vorrq(QRegister qd, QRegister qn, QRegister qm);
#endif
  void vornq(QRegister qd, QRegister qn, QRegister qm);
#if 0
  // Moved to Arm32::AssemblerARM32::vandq().
  void vandq(QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vandq().
  void vmvnq(QRegister qd, QRegister qm);

  // Moved to Arm32::AssemblerARM32::vceqqi().
  void vceqqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vceqqs().
  void vceqqs(QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcgeqi().
  void vcgeqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcugeqi().
  void vcugeqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcgeqs().
  void vcgeqs(QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcgtqi().
  void vcgtqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcugtqi().
  void vcugtqi(OperandSize sz, QRegister qd, QRegister qn, QRegister qm);
  // Moved to Arm32::AssemblerARM32::vcgtqs().
  void vcgtqs(QRegister qd, QRegister qn, QRegister qm);

  // Moved to Arm32::AssemblerARM32::vabss().
  void vabss(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vabsd().
  void vabsd(DRegister dd, DRegister dm, Condition cond = AL);
  // Moved to Arm32::AssemblerARM32::vabsq().
  void vabsqs(QRegister qd, QRegister qm);
#endif
  void vnegs(SRegister sd, SRegister sm, Condition cond = AL);
  void vnegd(DRegister dd, DRegister dm, Condition cond = AL);
#if 0
  // Moved to ARM32::AssemblerARM32::vnegqs().
  void vnegqs(QRegister qd, QRegister qm);
  // Moved to ARM32::AssemblerARM32::vsqrts().
  void vsqrts(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vsqrts().
  void vsqrtd(DRegister dd, DRegister dm, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::vcvtsd().
  void vcvtsd(SRegister sd, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32:vcvtds().
  void vcvtds(DRegister dd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtis()
  void vcvtis(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtid()
  void vcvtid(SRegister sd, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtsi()
  void vcvtsi(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtdi()
  void vcvtdi(DRegister dd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtus().
  void vcvtus(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtud().
  void vcvtud(SRegister sd, DRegister dm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtsu()
  void vcvtsu(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::vcvtdu()
  void vcvtdu(DRegister dd, SRegister sm, Condition cond = AL);

  // Moved to ARM23::AssemblerARM32::vcmps().
  void vcmps(SRegister sd, SRegister sm, Condition cond = AL);
  // Moved to ARM23::AssemblerARM32::vcmpd().
  void vcmpd(DRegister dd, DRegister dm, Condition cond = AL);
  // Moved to ARM23::AssemblerARM32::vcmpsz().
  void vcmpsz(SRegister sd, Condition cond = AL);
  // Moved to ARM23::AssemblerARM32::vcmpdz().
  void vcmpdz(DRegister dd, Condition cond = AL);

  // APSR_nzcv version moved to ARM32::AssemblerARM32::vmrsAPSR_nzcv()
  void vmrs(Register rd, Condition cond = AL);
#endif
  void vmstat(Condition cond = AL);

  // Duplicates the operand of size sz at index idx from dm to all elements of
  // qd. This is a special case of vtbl.
  void vdup(OperandSize sz, QRegister qd, DRegister dm, int idx);

  // Each byte of dm is an index into the table of bytes formed by concatenating
  // a list of 'length' registers starting with dn. The result is placed in dd.
  void vtbl(DRegister dd, DRegister dn, int length, DRegister dm);

  // The words of qd and qm are interleaved with the low words of the result
  // in qd and the high words in qm.
  void vzipqw(QRegister qd, QRegister qm);

  // Branch instructions.
#if 0
  // Moved to ARM32::AssemblerARM32::b();
  void b(Label* label, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::bl()
  void bl(Label* label, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::bx()
  void bx(Register rm, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::blx()
  void blx(Register rm, Condition cond = AL);
#endif

  void Branch(const StubEntry &stub_entry,
              Patchability patchable = kNotPatchable, Register pp = PP,
              Condition cond = AL);

  void BranchLink(const StubEntry &stub_entry,
                  Patchability patchable = kNotPatchable);
  void BranchLink(const Code &code, Patchability patchable);

  // Branch and link to an entry address. Call sequence can be patched.
  void BranchLinkPatchable(const StubEntry &stub_entry);
  void BranchLinkPatchable(const Code &code);

  // Branch and link to [base + offset]. Call sequence is never patched.
  void BranchLinkOffset(Register base, int32_t offset);

  // Add signed immediate value to rd. May clobber IP.
  void AddImmediate(Register rd, int32_t value, Condition cond = AL);
  void AddImmediate(Register rd, Register rn, int32_t value,
                    Condition cond = AL);
  void AddImmediateSetFlags(Register rd, Register rn, int32_t value,
                            Condition cond = AL);
  void SubImmediateSetFlags(Register rd, Register rn, int32_t value,
                            Condition cond = AL);
  void AndImmediate(Register rd, Register rs, int32_t imm, Condition cond = AL);

  // Test rn and immediate. May clobber IP.
  void TestImmediate(Register rn, int32_t imm, Condition cond = AL);

  // Compare rn with signed immediate value. May clobber IP.
  void CompareImmediate(Register rn, int32_t value, Condition cond = AL);

  // Signed integer division of left by right. Checks to see if integer
  // division is supported. If not, uses the FPU for division with
  // temporary registers tmpl and tmpr. tmpl and tmpr must be different
  // registers.
  void IntegerDivide(Register result, Register left, Register right,
                     DRegister tmpl, DRegister tmpr);

  // Load and Store.
  // These three do not clobber IP.
  void LoadPatchableImmediate(Register rd, int32_t value, Condition cond = AL);
  void LoadDecodableImmediate(Register rd, int32_t value, Condition cond = AL);
  void LoadImmediate(Register rd, int32_t value, Condition cond = AL);
  // These two may clobber IP.
  void LoadSImmediate(SRegister sd, float value, Condition cond = AL);
  void LoadDImmediate(DRegister dd, double value, Register scratch,
                      Condition cond = AL);

  void MarkExceptionHandler(Label *label);

  void Drop(intptr_t stack_elements);

  void RestoreCodePointer();
  void LoadPoolPointer(Register reg = PP);

  void LoadIsolate(Register rd);

  void LoadObject(Register rd, const Object &object, Condition cond = AL);
  void LoadUniqueObject(Register rd, const Object &object, Condition cond = AL);
  void LoadFunctionFromCalleePool(Register dst, const Function &function,
                                  Register new_pp);
  void LoadNativeEntry(Register dst, const ExternalLabel *label,
                       Patchability patchable, Condition cond = AL);
  void PushObject(const Object &object);
  void CompareObject(Register rn, const Object &object);

  // When storing into a heap object field, knowledge of the previous content
  // is expressed through these constants.
  enum FieldContent {
    kEmptyOrSmiOrNull, // Empty = garbage/zapped in release/debug mode.
    kHeapObjectOrSmi,
    kOnlySmi,
  };

  void StoreIntoObject(Register object,     // Object we are storing into.
                       const Address &dest, // Where we are storing into.
                       Register value,      // Value we are storing.
                       bool can_value_be_smi = true);
  void StoreIntoObjectOffset(Register object, int32_t offset, Register value,
                             bool can_value_be_smi = true);

  void StoreIntoObjectNoBarrier(Register object, const Address &dest,
                                Register value,
                                FieldContent old_content = kHeapObjectOrSmi);
  void InitializeFieldNoBarrier(Register object, const Address &dest,
                                Register value) {
    StoreIntoObjectNoBarrier(object, dest, value, kEmptyOrSmiOrNull);
  }
  void
  StoreIntoObjectNoBarrierOffset(Register object, int32_t offset,
                                 Register value,
                                 FieldContent old_content = kHeapObjectOrSmi);
  void StoreIntoObjectNoBarrier(Register object, const Address &dest,
                                const Object &value,
                                FieldContent old_content = kHeapObjectOrSmi);
  void
  StoreIntoObjectNoBarrierOffset(Register object, int32_t offset,
                                 const Object &value,
                                 FieldContent old_content = kHeapObjectOrSmi);

  // Store value_even, value_odd, value_even, ... into the words in the address
  // range [begin, end), assumed to be uninitialized fields in object (tagged).
  // The stores must not need a generational store barrier (e.g., smi/null),
  // and (value_even, value_odd) must be a valid register pair.
  // Destroys register 'begin'.
  void InitializeFieldsNoBarrier(Register object, Register begin, Register end,
                                 Register value_even, Register value_odd);
  // Like above, for the range [base+begin_offset, base+end_offset), unrolled.
  void InitializeFieldsNoBarrierUnrolled(Register object, Register base,
                                         intptr_t begin_offset,
                                         intptr_t end_offset,
                                         Register value_even,
                                         Register value_odd);

  // Stores a Smi value into a heap object field that always contains a Smi.
  void StoreIntoSmiField(const Address &dest, Register value);

  void LoadClassId(Register result, Register object, Condition cond = AL);
  void LoadClassById(Register result, Register class_id);
  void LoadClass(Register result, Register object, Register scratch);
  void CompareClassId(Register object, intptr_t class_id, Register scratch);
  void LoadClassIdMayBeSmi(Register result, Register object);
  void LoadTaggedClassIdMayBeSmi(Register result, Register object);

  void ComputeRange(Register result, Register value, Register scratch,
                    Label *miss);

  void UpdateRangeFeedback(Register value, intptr_t idx, Register ic_data,
                           Register scratch1, Register scratch2, Label *miss);

  intptr_t FindImmediate(int32_t imm);
  bool CanLoadFromObjectPool(const Object &object) const;
  void LoadFromOffset(OperandSize type, Register reg, Register base,
                      int32_t offset, Condition cond = AL);
  void LoadFieldFromOffset(OperandSize type, Register reg, Register base,
                           int32_t offset, Condition cond = AL) {
    LoadFromOffset(type, reg, base, offset - kHeapObjectTag, cond);
  }
  void StoreToOffset(OperandSize type, Register reg, Register base,
                     int32_t offset, Condition cond = AL);
  void LoadSFromOffset(SRegister reg, Register base, int32_t offset,
                       Condition cond = AL);
  void StoreSToOffset(SRegister reg, Register base, int32_t offset,
                      Condition cond = AL);
  void LoadDFromOffset(DRegister reg, Register base, int32_t offset,
                       Condition cond = AL);
  void StoreDToOffset(DRegister reg, Register base, int32_t offset,
                      Condition cond = AL);

  void LoadMultipleDFromOffset(DRegister first, intptr_t count, Register base,
                               int32_t offset);
  void StoreMultipleDToOffset(DRegister first, intptr_t count, Register base,
                              int32_t offset);

  void CopyDoubleField(Register dst, Register src, Register tmp1, Register tmp2,
                       DRegister dtmp);
  void CopyFloat32x4Field(Register dst, Register src, Register tmp1,
                          Register tmp2, DRegister dtmp);
  void CopyFloat64x2Field(Register dst, Register src, Register tmp1,
                          Register tmp2, DRegister dtmp);

#if 0
  // Moved to ARM32::AssemblerARM32::push().
  void Push(Register rd, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::pop().
  void Pop(Register rd, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::pushList().
  void PushList(RegList regs, Condition cond = AL);

  // Moved to ARM32::AssemblerARM32::popList().
  void PopList(RegList regs, Condition cond = AL);
#endif
  void MoveRegister(Register rd, Register rm, Condition cond = AL);

  // Convenience shift instructions. Use mov instruction with shifter operand
  // for variants setting the status flags.
#if 0
  // Moved to ARM32::AssemblerARM32::lsl()
  void Lsl(Register rd, Register rm, const Operand& shift_imm,
           Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::lsl()
  void Lsl(Register rd, Register rm, Register rs, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::lsr()
  void Lsr(Register rd, Register rm, const Operand& shift_imm,
           Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::lsr()
  void Lsr(Register rd, Register rm, Register rs, Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::asr()
  void Asr(Register rd, Register rm, const Operand& shift_imm,
           Condition cond = AL);
  // Moved to ARM32::AssemblerARM32::asr()
  void Asr(Register rd, Register rm, Register rs, Condition cond = AL);
#endif
  void Asrs(Register rd, Register rm, const Operand &shift_imm,
            Condition cond = AL);
  void Ror(Register rd, Register rm, const Operand &shift_imm,
           Condition cond = AL);
  void Ror(Register rd, Register rm, Register rs, Condition cond = AL);
  void Rrx(Register rd, Register rm, Condition cond = AL);

  // Fill rd with the sign of rm.
  void SignFill(Register rd, Register rm, Condition cond = AL);

  void Vreciprocalqs(QRegister qd, QRegister qm);
  void VreciprocalSqrtqs(QRegister qd, QRegister qm);
  // If qm must be preserved, then provide a (non-QTMP) temporary.
  void Vsqrtqs(QRegister qd, QRegister qm, QRegister temp);
  void Vdivqs(QRegister qd, QRegister qn, QRegister qm);

  void SmiTag(Register reg, Condition cond = AL) {
    Lsl(reg, reg, Operand(kSmiTagSize), cond);
  }

  void SmiTag(Register dst, Register src, Condition cond = AL) {
    Lsl(dst, src, Operand(kSmiTagSize), cond);
  }

  void SmiUntag(Register reg, Condition cond = AL) {
    Asr(reg, reg, Operand(kSmiTagSize), cond);
  }

  void SmiUntag(Register dst, Register src, Condition cond = AL) {
    Asr(dst, src, Operand(kSmiTagSize), cond);
  }

  // Untag the value in the register assuming it is a smi.
  // Untagging shifts tag bit into the carry flag - if carry is clear
  // assumption was correct. In this case jump to the is_smi label.
  // Otherwise fall-through.
  void SmiUntag(Register dst, Register src, Label *is_smi) {
    ASSERT(kSmiTagSize == 1);
    Asrs(dst, src, Operand(kSmiTagSize));
    b(is_smi, CC);
  }

  void CheckCodePointer();

  // Function frame setup and tear down.
  void EnterFrame(RegList regs, intptr_t frame_space);
  void LeaveFrame(RegList regs);
  void Ret();
  void ReserveAlignedFrameSpace(intptr_t frame_space);

  // Create a frame for calling into runtime that preserves all volatile
  // registers.  Frame's SP is guaranteed to be correctly aligned and
  // frame_space bytes are reserved under it.
  void EnterCallRuntimeFrame(intptr_t frame_space);
  void LeaveCallRuntimeFrame();

  void CallRuntime(const RuntimeEntry &entry, intptr_t argument_count);

  // Set up a Dart frame on entry with a frame pointer and PC information to
  // enable easy access to the RawInstruction object of code corresponding
  // to this frame.
  void EnterDartFrame(intptr_t frame_size);
  void LeaveDartFrame(RestorePP restore_pp = kRestoreCallerPP);

  // Set up a Dart frame for a function compiled for on-stack replacement.
  // The frame layout is a normal Dart frame, but the frame is partially set
  // up on entry (it is the frame of the unoptimized code).
  void EnterOsrFrame(intptr_t extra_size);

  // Set up a stub frame so that the stack traversal code can easily identify
  // a stub frame.
  void EnterStubFrame();
  void LeaveStubFrame();

  // The register into which the allocation stats table is loaded with
  // LoadAllocationStatsAddress should be passed to
  // IncrementAllocationStats(WithSize) as stats_addr_reg to update the
  // allocation stats. These are separate assembler macros so we can
  // avoid a dependent load too nearby the load of the table address.
  void LoadAllocationStatsAddress(Register dest, intptr_t cid,
                                  bool inline_isolate = true);
  void IncrementAllocationStats(Register stats_addr, intptr_t cid,
                                Heap::Space space);
  void IncrementAllocationStatsWithSize(Register stats_addr_reg,
                                        Register size_reg, Heap::Space space);

  Address ElementAddressForIntIndex(bool is_load, bool is_external,
                                    intptr_t cid, intptr_t index_scale,
                                    Register array, intptr_t index,
                                    Register temp);

  Address ElementAddressForRegIndex(bool is_load, bool is_external,
                                    intptr_t cid, intptr_t index_scale,
                                    Register array, Register index);

  // If allocation tracing for |cid| is enabled, will jump to |trace| label,
  // which will allocate in the runtime where tracing occurs.
  void MaybeTraceAllocation(intptr_t cid, Register temp_reg, Label *trace,
                            bool inline_isolate = true);

  // Inlined allocation of an instance of class 'cls', code has no runtime
  // calls. Jump to 'failure' if the instance cannot be allocated here.
  // Allocated instance is returned in 'instance_reg'.
  // Only the tags field of the object is initialized.
  void TryAllocate(const Class &cls, Label *failure, Register instance_reg,
                   Register temp_reg);

  void TryAllocateArray(intptr_t cid, intptr_t instance_size, Label *failure,
                        Register instance, Register end_address, Register temp1,
                        Register temp2);

  // Emit data (e.g encoded instruction or immediate) in instruction stream.
  void Emit(int32_t value);

  // On some other platforms, we draw a distinction between safe and unsafe
  // smis.
  static bool IsSafe(const Object &object) { return true; }
  static bool IsSafeSmi(const Object &object) { return object.IsSmi(); }

  bool constant_pool_allowed() const { return constant_pool_allowed_; }
  void set_constant_pool_allowed(bool b) { constant_pool_allowed_ = b; }

private:
  AssemblerBuffer buffer_; // Contains position independent code.
  ObjectPoolWrapper object_pool_wrapper_;

  int32_t prologue_offset_;

  bool use_far_branches_;

#if 0
  // If you are thinking of using one or both of these instructions directly,
  // instead LoadImmediate should probably be used.
  // Moved to ARM::AssemblerARM32::movw
  void movw(Register rd, uint16_t imm16, Condition cond = AL);
  // Moved to ARM::AssemblerARM32::movt
  void movt(Register rd, uint16_t imm16, Condition cond = AL);
#endif

  void BindARMv6(Label *label);
  void BindARMv7(Label *label);

  void LoadWordFromPoolOffset(Register rd, int32_t offset, Register pp,
                              Condition cond);

  void BranchLink(const ExternalLabel *label);

  class CodeComment : public ZoneAllocated {
  public:
    CodeComment(intptr_t pc_offset, const String &comment)
        : pc_offset_(pc_offset), comment_(comment) {}

    intptr_t pc_offset() const { return pc_offset_; }
    const String &comment() const { return comment_; }

  private:
    intptr_t pc_offset_;
    const String &comment_;

    DISALLOW_COPY_AND_ASSIGN(CodeComment);
  };

  GrowableArray<CodeComment *> comments_;

  bool constant_pool_allowed_;

  void LoadObjectHelper(Register rd, const Object &object, Condition cond,
                        bool is_unique, Register pp);

#if 0
  // Moved to ARM32::AssemblerARM32::emitType01()
  void EmitType01(Condition cond,
                  int type,
                  Opcode opcode,
                  int set_cc,
                  Register rn,
                  Register rd,
                  Operand o);

  // Moved to ARM32::AssemblerARM32::emitType05()
  void EmitType5(Condition cond, int32_t offset, bool link);

  // Moved to ARM32::AssemberARM32::emitMemOp()
  void EmitMemOp(Condition cond,
                 bool load,
                 bool byte,
                 Register rd,
                 Address ad);

  // Moved to AssemblerARM32::emitMemOpEnc3();
  void EmitMemOpAddressMode3(Condition cond,
                             int32_t mode,
                             Register rd,
                             Address ad);

  // Moved to ARM32::AssemblerARM32::emitMultiMemOp()
  void EmitMultiMemOp(Condition cond,
                      BlockAddressMode am,
                      bool load,
                      Register base,
                      RegList regs);
#endif

  void EmitShiftImmediate(Condition cond, Shift opcode, Register rd,
                          Register rm, Operand o);

  void EmitShiftRegister(Condition cond, Shift opcode, Register rd, Register rm,
                         Operand o);

#if 0
  // Moved to ARM32::AssemblerARM32::emitMulOp()
  void EmitMulOp(Condition cond,
                 int32_t opcode,
                 Register rd,
                 Register rn,
                 Register rm,
                 Register rs);

  // Moved to ARM32::AssemblerARM32::emitDivOp();
  void EmitDivOp(Condition cond,
                 int32_t opcode,
                 Register rd,
                 Register rn,
                 Register rm);
#endif

  void EmitMultiVSMemOp(Condition cond, BlockAddressMode am, bool load,
                        Register base, SRegister start, uint32_t count);

  void EmitMultiVDMemOp(Condition cond, BlockAddressMode am, bool load,
                        Register base, DRegister start, int32_t count);

#if 0
  // Moved to ARM32::AssemblerARM32::emitVFPsss
  void EmitVFPsss(Condition cond,
                  int32_t opcode,
                  SRegister sd,
                  SRegister sn,
                  SRegister sm);

  // Moved to ARM32::AssemblerARM32::emitVFPddd
  void EmitVFPddd(Condition cond,
                  int32_t opcode,
                  DRegister dd,
                  DRegister dn,
                  DRegister dm);

  // Moved to ARM32::AssemblerARM32::emitVFPsd
  void EmitVFPsd(Condition cond,
                 int32_t opcode,
                 SRegister sd,
                 DRegister dm);

  // Moved to ARM32::AssemblerARM32::emitVFPds
  void EmitVFPds(Condition cond,
                 int32_t opcode,
                 DRegister dd,
                 SRegister sm);

  // Moved to ARM32::AssemblerARM32::emitSIMDqqq()
  void EmitSIMDqqq(int32_t opcode, OperandSize sz,
                   QRegister qd, QRegister qn, QRegister qm);
#endif

  void EmitSIMDddd(int32_t opcode, OperandSize sz, DRegister dd, DRegister dn,
                   DRegister dm);

  void EmitFarBranch(Condition cond, int32_t offset, bool link);
#if 0
  // Moved to ARM32::AssemblerARM32::emitBranch()
  void EmitBranch(Condition cond, Label* label, bool link);
  // Moved to ARM32::AssemblerARM32::encodeBranchoffset().
  int32_t EncodeBranchOffset(int32_t offset, int32_t inst);
  // Moved to ARM32::AssemberARM32::decodeBranchOffset().
  static int32_t DecodeBranchOffset(int32_t inst);
#endif
  int32_t EncodeTstOffset(int32_t offset, int32_t inst);
  int32_t DecodeTstOffset(int32_t inst);

  void StoreIntoObjectFilter(Register object, Register value, Label *no_update);

  // Shorter filtering sequence that assumes that value is not a smi.
  void StoreIntoObjectFilterNoSmi(Register object, Register value,
                                  Label *no_update);

  // Helpers for write-barrier verification.

  // Returns VerifiedMemory::offset() as an Operand.
  Operand GetVerifiedMemoryShadow();
  // Writes value to [base + offset] and also its shadow location, if enabled.
  void WriteShadowedField(Register base, intptr_t offset, Register value,
                          Condition cond = AL);
  void WriteShadowedFieldPair(Register base, intptr_t offset,
                              Register value_even, Register value_odd,
                              Condition cond = AL);
  // Writes new_value to address and its shadow location, if enabled, after
  // verifying that its old value matches its shadow.
  void VerifiedWrite(const Address &address, Register new_value,
                     FieldContent old_content);

#if 0
  // Added the following missing operations:
  //
  // ARM32::AssemblerARM32::uxt() (uxtb and uxth)
  // ARM32::AssemblerARM32::vpop()
  // ARM32::AssemblerARM32::vpush()
  // ARM32::AssemblerARM32::rbit()
  // ARM32::AssemblerARM32::vbslq()
  // ARM32::AssemblerARM32::veord()
  // ARM32::AssemblerARM32::vld1qr()
  // ARM32::AssemblerARM32::vshlqc
  // ARM32::AssemblerARM32::vshrqic
  // ARM32::AssemblerARM32::vshrquc
  // ARM32::AssemblerARM32::vst1qr()
  // ARM32::AssemblerARM32::vmorqi()
  // ARM32::AssemblerARM32::vmovqc()
#endif

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(Assembler);
};

} // namespace dart

#endif // VM_ASSEMBLER_ARM_H_
