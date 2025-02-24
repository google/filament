//===- subzero/unittest/AssemblerX8664/LowLevel.cpp -----------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AssemblerX8664/TestUtil.h"

namespace Ice {
namespace X8664 {
namespace Test {
namespace {

TEST_F(AssemblerX8664LowLevelTest, Ret) {
  __ ret();

  constexpr size_t ByteCount = 1;
  ASSERT_EQ(ByteCount, codeBytesSize());

  verifyBytes<ByteCount>(codeBytes(), 0xc3);
}

TEST_F(AssemblerX8664LowLevelTest, RetImm) {
  __ ret(Immediate(0x20));

  constexpr size_t ByteCount = 3;
  ASSERT_EQ(ByteCount, codeBytesSize());

  verifyBytes<ByteCount>(codeBytes(), 0xC2, 0x20, 0x00);
}

TEST_F(AssemblerX8664LowLevelTest, CallImm4) {
  __ call(Immediate(4));

  constexpr size_t ByteCount = 5;
  ASSERT_EQ(ByteCount, codeBytesSize());

  verifyBytes<ByteCount>(codeBytes(), 0xe8, 0x00, 0x00, 0x00, 0x00);
}

TEST_F(AssemblerX8664LowLevelTest, PopRegs) {
  __ popl(Encoded_GPR_eax());
  __ popl(Encoded_GPR_ebx());
  __ popl(Encoded_GPR_ecx());
  __ popl(Encoded_GPR_edx());
  __ popl(Encoded_GPR_edi());
  __ popl(Encoded_GPR_esi());
  __ popl(Encoded_GPR_ebp());
  __ popl(Encoded_GPR_r8());
  __ popl(Encoded_GPR_r9());
  __ popl(Encoded_GPR_r10());
  __ popl(Encoded_GPR_r11());
  __ popl(Encoded_GPR_r12());
  __ popl(Encoded_GPR_r13());
  __ popl(Encoded_GPR_r14());
  __ popl(Encoded_GPR_r15());

  constexpr size_t ByteCount = 23;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Rex_B = 0x41;
  constexpr uint8_t PopOpcode = 0x58;
  verifyBytes<ByteCount>(
      codeBytes(), PopOpcode | Encoded_GPR_eax(), PopOpcode | Encoded_GPR_ebx(),
      PopOpcode | Encoded_GPR_ecx(), PopOpcode | Encoded_GPR_edx(),
      PopOpcode | Encoded_GPR_edi(), PopOpcode | Encoded_GPR_esi(),
      PopOpcode | Encoded_GPR_ebp(), Rex_B, PopOpcode | (Encoded_GPR_r8() & 7),
      Rex_B, PopOpcode | (Encoded_GPR_r9() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r10() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r11() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r12() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r13() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r14() & 7), Rex_B,
      PopOpcode | (Encoded_GPR_r15() & 7));
}

TEST_F(AssemblerX8664LowLevelTest, PushRegs) {
  __ pushl(Encoded_GPR_eax());
  __ pushl(Encoded_GPR_ebx());
  __ pushl(Encoded_GPR_ecx());
  __ pushl(Encoded_GPR_edx());
  __ pushl(Encoded_GPR_edi());
  __ pushl(Encoded_GPR_esi());
  __ pushl(Encoded_GPR_ebp());
  __ pushl(Encoded_GPR_r8());
  __ pushl(Encoded_GPR_r9());
  __ pushl(Encoded_GPR_r10());
  __ pushl(Encoded_GPR_r11());
  __ pushl(Encoded_GPR_r12());
  __ pushl(Encoded_GPR_r13());
  __ pushl(Encoded_GPR_r14());
  __ pushl(Encoded_GPR_r15());

  constexpr size_t ByteCount = 23;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Rex_B = 0x41;
  constexpr uint8_t PushOpcode = 0x50;
  verifyBytes<ByteCount>(
      codeBytes(), PushOpcode | Encoded_GPR_eax(),
      PushOpcode | Encoded_GPR_ebx(), PushOpcode | Encoded_GPR_ecx(),
      PushOpcode | Encoded_GPR_edx(), PushOpcode | Encoded_GPR_edi(),
      PushOpcode | Encoded_GPR_esi(), PushOpcode | Encoded_GPR_ebp(), Rex_B,
      PushOpcode | (Encoded_GPR_r8() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r9() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r10() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r11() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r12() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r13() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r14() & 7), Rex_B,
      PushOpcode | (Encoded_GPR_r15() & 7));
}

TEST_F(AssemblerX8664LowLevelTest, MovRegisterZero) {
  __ mov(IceType_i32, Encoded_GPR_eax(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_ebx(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_ecx(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_edx(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_edi(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_esi(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_ebp(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r8(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r10(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r11(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r12(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r13(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r14(), Immediate(0x00));
  __ mov(IceType_i32, Encoded_GPR_r15(), Immediate(0x00));

  constexpr uint8_t Rex_B = 0x41;
  constexpr size_t MovReg32BitImmBytes = 5;
  constexpr size_t ByteCount = 14 * MovReg32BitImmBytes + 7 /*Rex_B*/;

  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t MovOpcode = 0xb8;
  verifyBytes<ByteCount>(
      codeBytes(), MovOpcode | Encoded_GPR_eax(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_ebx(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_ecx(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_edx(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_edi(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_esi(), 0x00, 0x00, 0x00, 0x00,
      MovOpcode | Encoded_GPR_ebp(), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r8() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r10() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r11() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r12() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r13() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r14() & 7), 0x00, 0x00, 0x00, 0x00, Rex_B,
      MovOpcode | (Encoded_GPR_r15() & 7), 0x00, 0x00, 0x00, 0x00);
}

TEST_F(AssemblerX8664LowLevelTest, Cmp) {
#define TestRegReg(Inst, Dst, Src, OpType, ByteCountUntyped, ...)              \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Src ", " #OpType ", " #ByteCountUntyped      \
        ",  " #__VA_ARGS__ ")";                                                \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(), Encoded_GPR_##Src());       \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestRegImm(Inst, Dst, Imm, OpType, ByteCountUntyped, ...)              \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Imm ", " #OpType ", " #ByteCountUntyped      \
        ",  " #__VA_ARGS__ ")";                                                \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(), Immediate(Imm));            \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestRegAbsoluteAddr(Inst, Dst, Disp, OpType, ByteCountUntyped, ...)    \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Disp ", " #OpType ", " #ByteCountUntyped     \
        ",  " #__VA_ARGS__ ")";                                                \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(), Address::Absolute(Disp));   \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestRegAddrBase(Inst, Dst, Base, Disp, OpType, ByteCountUntyped, ...)  \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Base ", " #Disp ", " #OpType                 \
        ", " #ByteCountUntyped ",  " #__VA_ARGS__ ")";                         \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(),                             \
            Address(Encoded_GPR_##Base(), Disp, AssemblerFixup::NoFixup));     \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestRegAddrScaledIndex(Inst, Dst, Index, Scale, Disp, OpType,          \
                               ByteCountUntyped, ...)                          \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Index ", " #Scale ", " #Disp ", " #OpType    \
        ", " #ByteCountUntyped ",  " #__VA_ARGS__ ")";                         \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(),                             \
            Address(Encoded_GPR_##Index(), Traits::TIMES_##Scale, Disp,        \
                    AssemblerFixup::NoFixup));                                 \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestRegAddrBaseScaledIndex(Inst, Dst, Base, Index, Scale, Disp,        \
                                   OpType, ByteCountUntyped, ...)              \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Base ", " #Index ", " #Scale ", " #Disp      \
        ", " #OpType ", " #ByteCountUntyped ",  " #__VA_ARGS__ ")";            \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType, Encoded_GPR_##Dst(),                             \
            Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),               \
                    Traits::TIMES_##Scale, Disp, AssemblerFixup::NoFixup));    \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestAddrBaseScaledIndexImm(Inst, Base, Index, Scale, Disp, Imm,        \
                                   OpType, ByteCountUntyped, ...)              \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Base ", " #Index ", " #Scale ", " #Disp ", " #Imm      \
        ", " #OpType ", " #ByteCountUntyped ",  " #__VA_ARGS__ ")";            \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType,                                                  \
            Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),               \
                    Traits::TIMES_##Scale, Disp, AssemblerFixup::NoFixup),     \
            Immediate(Imm));                                                   \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestAddrBaseScaledIndexReg(Inst, Base, Index, Scale, Disp, Src,        \
                                   OpType, ByteCountUntyped, ...)              \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Base ", " #Index ", " #Scale ", " #Disp ", " #Src      \
        ", " #OpType ", " #ByteCountUntyped ",  " #__VA_ARGS__ ")";            \
    static constexpr uint8_t ByteCount = ByteCountUntyped;                     \
    __ Inst(IceType_##OpType,                                                  \
            Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),               \
                    Traits::TIMES_##Scale, Disp, AssemblerFixup::NoFixup),     \
            Encoded_GPR_##Src());                                              \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), __VA_ARGS__))              \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

  /* cmp GPR, GPR */
  TestRegReg(cmp, eax, ecx, i32, 2, 0x3B, 0xC1);
  TestRegReg(cmp, ecx, edx, i32, 2, 0x3B, 0xCA);
  TestRegReg(cmp, edx, ebx, i32, 2, 0x3B, 0xD3);
  TestRegReg(cmp, ebx, esp, i32, 2, 0x3B, 0xDC);
  TestRegReg(cmp, esp, ebp, i32, 2, 0x3B, 0xE5);
  TestRegReg(cmp, ebp, esi, i32, 2, 0x3B, 0xEE);
  TestRegReg(cmp, esi, edi, i32, 2, 0x3B, 0xF7);
  TestRegReg(cmp, edi, r8, i32, 3, 0x41, 0x3B, 0xF8);
  TestRegReg(cmp, r8, r9, i32, 3, 0x45, 0x3B, 0xC1);
  TestRegReg(cmp, r9, r10, i32, 3, 0x45, 0x3B, 0xCA);
  TestRegReg(cmp, r10, r11, i32, 3, 0x45, 0x3B, 0xD3);
  TestRegReg(cmp, r11, r12, i32, 3, 0x45, 0x3B, 0xDC);
  TestRegReg(cmp, r12, r13, i32, 3, 0x45, 0x3B, 0xE5);
  TestRegReg(cmp, r13, r14, i32, 3, 0x45, 0x3B, 0xEE);
  TestRegReg(cmp, r14, r15, i32, 3, 0x45, 0x3B, 0xF7);
  TestRegReg(cmp, r15, eax, i32, 3, 0x44, 0x3B, 0xF8);

  TestRegReg(cmp, eax, ecx, i16, 3, 0x66, 0x3B, 0xC1);
  TestRegReg(cmp, ecx, edx, i16, 3, 0x66, 0x3B, 0xCA);
  TestRegReg(cmp, edx, ebx, i16, 3, 0x66, 0x3B, 0xD3);
  TestRegReg(cmp, ebx, esp, i16, 3, 0x66, 0x3B, 0xDC);
  TestRegReg(cmp, esp, ebp, i16, 3, 0x66, 0x3B, 0xE5);
  TestRegReg(cmp, ebp, esi, i16, 3, 0x66, 0x3B, 0xEE);
  TestRegReg(cmp, esi, edi, i16, 3, 0x66, 0x3B, 0xF7);
  TestRegReg(cmp, edi, r8, i16, 4, 0x66, 0x41, 0x3B, 0xF8);
  TestRegReg(cmp, r8, r9, i16, 4, 0x66, 0x45, 0x3B, 0xC1);
  TestRegReg(cmp, r9, r10, i16, 4, 0x66, 0x45, 0x3B, 0xCA);
  TestRegReg(cmp, r10, r11, i16, 4, 0x66, 0x45, 0x3B, 0xD3);
  TestRegReg(cmp, r11, r12, i16, 4, 0x66, 0x45, 0x3B, 0xDC);
  TestRegReg(cmp, r12, r13, i16, 4, 0x66, 0x45, 0x3B, 0xE5);
  TestRegReg(cmp, r13, r14, i16, 4, 0x66, 0x45, 0x3B, 0xEE);
  TestRegReg(cmp, r14, r15, i16, 4, 0x66, 0x45, 0x3B, 0xF7);
  TestRegReg(cmp, r15, eax, i16, 4, 0x66, 0x44, 0x3B, 0xF8);

  TestRegReg(cmp, eax, ecx, i8, 2, 0x3A, 0xC1);
  TestRegReg(cmp, ecx, edx, i8, 2, 0x3A, 0xCA);
  TestRegReg(cmp, edx, ebx, i8, 2, 0x3A, 0xD3);
  TestRegReg(cmp, ebx, esp, i8, 2, 0x3A, 0xDC); // emit: cmp bl, ah
  TestRegReg(cmp, esp, ebp, i8, 3, 0x40, 0x3A, 0xE5);
  TestRegReg(cmp, ebp, esi, i8, 3, 0x40, 0x3A, 0xEE);
  TestRegReg(cmp, esi, edi, i8, 3, 0x40, 0x3A, 0xF7);
  TestRegReg(cmp, edi, r8, i8, 3, 0x41, 0x3A, 0xF8);
  TestRegReg(cmp, r8, r9, i8, 3, 0x45, 0x3A, 0xC1);
  TestRegReg(cmp, r9, r10, i8, 3, 0x45, 0x3A, 0xCA);
  TestRegReg(cmp, r10, r11, i8, 3, 0x45, 0x3A, 0xD3);
  TestRegReg(cmp, r11, r12, i8, 3, 0x45, 0x3A, 0xDC);
  TestRegReg(cmp, r12, r13, i8, 3, 0x45, 0x3A, 0xE5);
  TestRegReg(cmp, r13, r14, i8, 3, 0x45, 0x3A, 0xEE);
  TestRegReg(cmp, r14, r15, i8, 3, 0x45, 0x3A, 0xF7);
  TestRegReg(cmp, r15, eax, i8, 3, 0x44, 0x3A, 0xF8);

  /* cmp GPR, Imm8 */
  TestRegImm(cmp, eax, 5, i32, 3, 0x83, 0xF8, 0x05);
  TestRegImm(cmp, ecx, 5, i32, 3, 0x83, 0xF9, 0x05);
  TestRegImm(cmp, edx, 5, i32, 3, 0x83, 0xFA, 0x05);
  TestRegImm(cmp, ebx, 5, i32, 3, 0x83, 0xFB, 0x05);
  TestRegImm(cmp, esp, 5, i32, 3, 0x83, 0xFC, 0x05);
  TestRegImm(cmp, ebp, 5, i32, 3, 0x83, 0xFD, 0x05);
  TestRegImm(cmp, esi, 5, i32, 3, 0x83, 0xFE, 0x05);
  TestRegImm(cmp, edi, 5, i32, 3, 0x83, 0xFF, 0x05);
  TestRegImm(cmp, r8, 5, i32, 4, 0x41, 0x83, 0xF8, 0x05);
  TestRegImm(cmp, r9, 5, i32, 4, 0x41, 0x83, 0xF9, 0x05);
  TestRegImm(cmp, r10, 5, i32, 4, 0x41, 0x83, 0xFA, 0x05);
  TestRegImm(cmp, r11, 5, i32, 4, 0x41, 0x83, 0xFB, 0x05);
  TestRegImm(cmp, r12, 5, i32, 4, 0x41, 0x83, 0xFC, 0x05);
  TestRegImm(cmp, r13, 5, i32, 4, 0x41, 0x83, 0xFD, 0x05);
  TestRegImm(cmp, r14, 5, i32, 4, 0x41, 0x83, 0xFE, 0x05);
  TestRegImm(cmp, r15, 5, i32, 4, 0x41, 0x83, 0xFF, 0x05);

  TestRegImm(cmp, eax, 5, i16, 4, 0x66, 0x83, 0xF8, 0x05);
  TestRegImm(cmp, ecx, 5, i16, 4, 0x66, 0x83, 0xF9, 0x05);
  TestRegImm(cmp, edx, 5, i16, 4, 0x66, 0x83, 0xFA, 0x05);
  TestRegImm(cmp, ebx, 5, i16, 4, 0x66, 0x83, 0xFB, 0x05);
  TestRegImm(cmp, esp, 5, i16, 4, 0x66, 0x83, 0xFC, 0x05);
  TestRegImm(cmp, ebp, 5, i16, 4, 0x66, 0x83, 0xFD, 0x05);
  TestRegImm(cmp, esi, 5, i16, 4, 0x66, 0x83, 0xFE, 0x05);
  TestRegImm(cmp, edi, 5, i16, 4, 0x66, 0x83, 0xFF, 0x05);
  TestRegImm(cmp, r8, 5, i16, 5, 0x66, 0x41, 0x83, 0xF8, 0x05);
  TestRegImm(cmp, r9, 5, i16, 5, 0x66, 0x41, 0x83, 0xF9, 0x05);
  TestRegImm(cmp, r10, 5, i16, 5, 0x66, 0x41, 0x83, 0xFA, 0x05);
  TestRegImm(cmp, r11, 5, i16, 5, 0x66, 0x41, 0x83, 0xFB, 0x05);
  TestRegImm(cmp, r12, 5, i16, 5, 0x66, 0x41, 0x83, 0xFC, 0x05);
  TestRegImm(cmp, r13, 5, i16, 5, 0x66, 0x41, 0x83, 0xFD, 0x05);
  TestRegImm(cmp, r14, 5, i16, 5, 0x66, 0x41, 0x83, 0xFE, 0x05);
  TestRegImm(cmp, r15, 5, i16, 5, 0x66, 0x41, 0x83, 0xFF, 0x05);

  TestRegImm(cmp, eax, 5, i8, 2, 0x3C, 0x05);
  TestRegImm(cmp, ecx, 5, i8, 3, 0x80, 0xF9, 0x05);
  TestRegImm(cmp, edx, 5, i8, 3, 0x80, 0xFA, 0x05);
  TestRegImm(cmp, ebx, 5, i8, 3, 0x80, 0xFB, 0x05);
  TestRegImm(cmp, esp, 5, i8, 3, 0x80, 0xFC, 0x05); // emit: cmp ah, 5
  TestRegImm(cmp, ebp, 5, i8, 4, 0x40, 0x80, 0xFD, 0x05);
  TestRegImm(cmp, esi, 5, i8, 4, 0x40, 0x80, 0xFE, 0x05);
  TestRegImm(cmp, edi, 5, i8, 4, 0x40, 0x80, 0xFF, 0x05);
  TestRegImm(cmp, r8, 5, i8, 4, 0x41, 0x80, 0xF8, 0x05);
  TestRegImm(cmp, r9, 5, i8, 4, 0x41, 0x80, 0xF9, 0x05);
  TestRegImm(cmp, r10, 5, i8, 4, 0x41, 0x80, 0xFA, 0x05);
  TestRegImm(cmp, r11, 5, i8, 4, 0x41, 0x80, 0xFB, 0x05);
  TestRegImm(cmp, r12, 5, i8, 4, 0x41, 0x80, 0xFC, 0x05);
  TestRegImm(cmp, r13, 5, i8, 4, 0x41, 0x80, 0xFD, 0x05);
  TestRegImm(cmp, r14, 5, i8, 4, 0x41, 0x80, 0xFE, 0x05);
  TestRegImm(cmp, r15, 5, i8, 4, 0x41, 0x80, 0xFF, 0x05);

  /* cmp GPR, Imm16 */
  TestRegImm(cmp, eax, 0x100, i32, 5, 0x3D, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, ecx, 0x100, i32, 6, 0x81, 0xF9, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, edx, 0x100, i32, 6, 0x81, 0xFA, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, ebx, 0x100, i32, 6, 0x81, 0xFB, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, esp, 0x100, i32, 6, 0x81, 0xFC, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, ebp, 0x100, i32, 6, 0x81, 0xFD, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, esi, 0x100, i32, 6, 0x81, 0xFE, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, edi, 0x100, i32, 6, 0x81, 0xFF, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r8, 0x100, i32, 7, 0x41, 0x81, 0xF8, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r9, 0x100, i32, 7, 0x41, 0x81, 0xF9, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r10, 0x100, i32, 7, 0x41, 0x81, 0xFA, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r11, 0x100, i32, 7, 0x41, 0x81, 0xFB, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r12, 0x100, i32, 7, 0x41, 0x81, 0xFC, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r13, 0x100, i32, 7, 0x41, 0x81, 0xFD, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r14, 0x100, i32, 7, 0x41, 0x81, 0xFE, 0x00, 0x01, 0x00, 0x00);
  TestRegImm(cmp, r15, 0x100, i32, 7, 0x41, 0x81, 0xFF, 0x00, 0x01, 0x00, 0x00);

  TestRegImm(cmp, eax, 0x100, i16, 4, 0x66, 0x3D, 0x00, 0x01);
  TestRegImm(cmp, ecx, 0x100, i16, 5, 0x66, 0x81, 0xF9, 0x00, 0x01);
  TestRegImm(cmp, edx, 0x100, i16, 5, 0x66, 0x81, 0xFA, 0x00, 0x01);
  TestRegImm(cmp, ebx, 0x100, i16, 5, 0x66, 0x81, 0xFB, 0x00, 0x01);
  TestRegImm(cmp, esp, 0x100, i16, 5, 0x66, 0x81, 0xFC, 0x00, 0x01);
  TestRegImm(cmp, ebp, 0x100, i16, 5, 0x66, 0x81, 0xFD, 0x00, 0x01);
  TestRegImm(cmp, esi, 0x100, i16, 5, 0x66, 0x81, 0xFE, 0x00, 0x01);
  TestRegImm(cmp, edi, 0x100, i16, 5, 0x66, 0x81, 0xFF, 0x00, 0x01);
  TestRegImm(cmp, r8, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xF8, 0x00, 0x01);
  TestRegImm(cmp, r9, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xF9, 0x00, 0x01);
  TestRegImm(cmp, r10, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFA, 0x00, 0x01);
  TestRegImm(cmp, r11, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFB, 0x00, 0x01);
  TestRegImm(cmp, r12, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFC, 0x00, 0x01);
  TestRegImm(cmp, r13, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFD, 0x00, 0x01);
  TestRegImm(cmp, r14, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFE, 0x00, 0x01);
  TestRegImm(cmp, r15, 0x100, i16, 6, 0x66, 0x41, 0x81, 0xFF, 0x00, 0x01);

  /* cmp GPR, Absolute */
  TestRegAbsoluteAddr(cmp, eax, 0xF00FBEEF, i32, 8, 0x67, 0x3B, 0x04, 0x25,
                      0xEF, 0xBE, 0x0F, 0xF0);
  TestRegAbsoluteAddr(cmp, eax, 0xF00FBEEF, i16, 9, 0x66, 0x67, 0x3B, 0x04,
                      0x25, 0xEF, 0xBE, 0x0F, 0xF0);
  TestRegAbsoluteAddr(cmp, eax, 0xF00FBEEF, i8, 8, 0x67, 0x3A, 0x04, 0x25, 0xEF,
                      0xBE, 0x0F, 0xF0);
  TestRegAbsoluteAddr(cmp, r8, 0xF00FBEEF, i32, 9, 0x67, 0x44, 0x3B, 0x04, 0x25,
                      0xEF, 0xBE, 0x0F, 0xF0);
  TestRegAbsoluteAddr(cmp, r8, 0xF00FBEEF, i16, 10, 0x66, 0x67, 0x44, 0x3B,
                      0x04, 0x25, 0xEF, 0xBE, 0x0F, 0xF0);
  TestRegAbsoluteAddr(cmp, r8, 0xF00FBEEF, i8, 9, 0x67, 0x44, 0x3A, 0x04, 0x25,
                      0xEF, 0xBE, 0x0F, 0xF0);

  /* cmp GPR, 0(Base) */
  TestRegAddrBase(cmp, eax, ecx, 0, i32, 3, 0x67, 0x3B, 0x01);
  TestRegAddrBase(cmp, ecx, edx, 0, i32, 3, 0x67, 0x3B, 0x0A);
  TestRegAddrBase(cmp, edx, ebx, 0, i32, 3, 0x67, 0x3B, 0x13);
  TestRegAddrBase(cmp, ebx, esp, 0, i32, 4, 0x67, 0x3B, 0x1C, 0x24);
  TestRegAddrBase(cmp, esp, ebp, 0, i32, 4, 0x67, 0x3B, 0x65, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0, i32, 3, 0x67, 0x3B, 0x2E);
  TestRegAddrBase(cmp, esi, edi, 0, i32, 3, 0x67, 0x3B, 0x37);
  TestRegAddrBase(cmp, edi, r8, 0, i32, 4, 0x67, 0x41, 0x3B, 0x38);
  TestRegAddrBase(cmp, r8, r9, 0, i32, 4, 0x67, 0x45, 0x3B, 0x01);
  TestRegAddrBase(cmp, r9, r10, 0, i32, 4, 0x67, 0x45, 0x3B, 0x0A);
  TestRegAddrBase(cmp, r10, r11, 0, i32, 4, 0x67, 0x45, 0x3B, 0x13);
  TestRegAddrBase(cmp, r11, r12, 0, i32, 5, 0x67, 0x45, 0x3B, 0x1C, 0x24);
  TestRegAddrBase(cmp, r12, r13, 0, i32, 5, 0x67, 0x45, 0x3B, 0x65, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0, i32, 4, 0x67, 0x45, 0x3B, 0x2E);
  TestRegAddrBase(cmp, r14, r15, 0, i32, 4, 0x67, 0x45, 0x3B, 0x37);
  TestRegAddrBase(cmp, r15, eax, 0, i32, 4, 0x67, 0x44, 0x3B, 0x38);

  TestRegAddrBase(cmp, eax, ecx, 0, i16, 4, 0x66, 0x67, 0x3B, 0x01);
  TestRegAddrBase(cmp, ecx, edx, 0, i16, 4, 0x66, 0x67, 0x3B, 0x0A);
  TestRegAddrBase(cmp, edx, ebx, 0, i16, 4, 0x66, 0x67, 0x3B, 0x13);
  TestRegAddrBase(cmp, ebx, esp, 0, i16, 5, 0x66, 0x67, 0x3B, 0x1C, 0x24);
  TestRegAddrBase(cmp, esp, ebp, 0, i16, 5, 0x66, 0x67, 0x3B, 0x65, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0, i16, 4, 0x66, 0x67, 0x3B, 0x2E);
  TestRegAddrBase(cmp, esi, edi, 0, i16, 4, 0x66, 0x67, 0x3B, 0x37);
  TestRegAddrBase(cmp, edi, r8, 0, i16, 5, 0x66, 0x67, 0x41, 0x3B, 0x38);
  TestRegAddrBase(cmp, r8, r9, 0, i16, 5, 0x66, 0x67, 0x45, 0x3B, 0x01);
  TestRegAddrBase(cmp, r9, r10, 0, i16, 5, 0x66, 0x67, 0x45, 0x3B, 0x0A);
  TestRegAddrBase(cmp, r10, r11, 0, i16, 5, 0x66, 0x67, 0x45, 0x3B, 0x13);
  TestRegAddrBase(cmp, r11, r12, 0, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x1C, 0x24);
  TestRegAddrBase(cmp, r12, r13, 0, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x65, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0, i16, 5, 0x66, 0x67, 0x45, 0x3B, 0x2E);
  TestRegAddrBase(cmp, r14, r15, 0, i16, 5, 0x66, 0x67, 0x45, 0x3B, 0x37);
  TestRegAddrBase(cmp, r15, eax, 0, i16, 5, 0x66, 0x67, 0x44, 0x3B, 0x38);

  TestRegAddrBase(cmp, eax, ecx, 0, i8, 3, 0x67, 0x3A, 0x01);
  TestRegAddrBase(cmp, ecx, edx, 0, i8, 3, 0x67, 0x3A, 0x0A);
  TestRegAddrBase(cmp, edx, ebx, 0, i8, 3, 0x67, 0x3A, 0x13);
  TestRegAddrBase(cmp, ebx, esp, 0, i8, 4, 0x67, 0x3A, 0x1C, 0x24);
  TestRegAddrBase(cmp, esp, ebp, 0, i8, 4, 0x67, 0x3A, 0x65, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0, i8, 4, 0x67, 0x40, 0x3A, 0x2E);
  TestRegAddrBase(cmp, esi, edi, 0, i8, 4, 0x67, 0x40, 0x3A, 0x37);
  TestRegAddrBase(cmp, edi, r8, 0, i8, 4, 0x67, 0x41, 0x3A, 0x38);
  TestRegAddrBase(cmp, r8, r9, 0, i8, 4, 0x67, 0x45, 0x3A, 0x01);
  TestRegAddrBase(cmp, r9, r10, 0, i8, 4, 0x67, 0x45, 0x3A, 0x0A);
  TestRegAddrBase(cmp, r10, r11, 0, i8, 4, 0x67, 0x45, 0x3A, 0x13);
  TestRegAddrBase(cmp, r11, r12, 0, i8, 5, 0x67, 0x45, 0x3A, 0x1C, 0x24);
  TestRegAddrBase(cmp, r12, r13, 0, i8, 5, 0x67, 0x45, 0x3A, 0x65, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0, i8, 4, 0x67, 0x45, 0x3A, 0x2E);
  TestRegAddrBase(cmp, r14, r15, 0, i8, 4, 0x67, 0x45, 0x3A, 0x37);
  TestRegAddrBase(cmp, r15, eax, 0, i8, 4, 0x67, 0x44, 0x3A, 0x38);

  /* cmp GPR, Imm8(Base) */
  TestRegAddrBase(cmp, eax, ecx, 0x40, i32, 4, 0x67, 0x3B, 0x41, 0x40);
  TestRegAddrBase(cmp, ecx, edx, 0x40, i32, 4, 0x67, 0x3B, 0x4A, 0x40);
  TestRegAddrBase(cmp, edx, ebx, 0x40, i32, 4, 0x67, 0x3B, 0x53, 0x40);
  TestRegAddrBase(cmp, ebx, esp, 0x40, i32, 5, 0x67, 0x3B, 0x5C, 0x24, 0x40);
  TestRegAddrBase(cmp, esp, ebp, 0x40, i32, 4, 0x67, 0x3B, 0x65, 0x40);
  TestRegAddrBase(cmp, ebp, esi, 0x40, i32, 4, 0x67, 0x3B, 0x6E, 0x40);
  TestRegAddrBase(cmp, esi, edi, 0x40, i32, 4, 0x67, 0x3B, 0x77, 0x40);
  TestRegAddrBase(cmp, edi, r8, 0x40, i32, 5, 0x67, 0x41, 0x3B, 0x78, 0x40);
  TestRegAddrBase(cmp, r8, r9, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x41, 0x40);
  TestRegAddrBase(cmp, r9, r10, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x4A, 0x40);
  TestRegAddrBase(cmp, r10, r11, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x53, 0x40);
  TestRegAddrBase(cmp, r11, r12, 0x40, i32, 6, 0x67, 0x45, 0x3B, 0x5C, 0x24,
                  0x40);
  TestRegAddrBase(cmp, r12, r13, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x65, 0x40);
  TestRegAddrBase(cmp, r13, r14, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x6E, 0x40);
  TestRegAddrBase(cmp, r14, r15, 0x40, i32, 5, 0x67, 0x45, 0x3B, 0x77, 0x40);
  TestRegAddrBase(cmp, r15, eax, 0x40, i32, 5, 0x67, 0x44, 0x3B, 0x78, 0x40);

  TestRegAddrBase(cmp, eax, ecx, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x41, 0x40);
  TestRegAddrBase(cmp, ecx, edx, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x4A, 0x40);
  TestRegAddrBase(cmp, edx, ebx, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x53, 0x40);
  TestRegAddrBase(cmp, ebx, esp, 0x40, i16, 6, 0x66, 0x67, 0x3B, 0x5C, 0x24,
                  0x40);
  TestRegAddrBase(cmp, esp, ebp, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x65, 0x40);
  TestRegAddrBase(cmp, ebp, esi, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x6E, 0x40);
  TestRegAddrBase(cmp, esi, edi, 0x40, i16, 5, 0x66, 0x67, 0x3B, 0x77, 0x40);
  TestRegAddrBase(cmp, edi, r8, 0x40, i16, 6, 0x66, 0x67, 0x41, 0x3B, 0x78,
                  0x40);
  TestRegAddrBase(cmp, r8, r9, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x41,
                  0x40);
  TestRegAddrBase(cmp, r9, r10, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x4A,
                  0x40);
  TestRegAddrBase(cmp, r10, r11, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x53,
                  0x40);
  TestRegAddrBase(cmp, r11, r12, 0x40, i16, 7, 0x66, 0x67, 0x45, 0x3B, 0x5C,
                  0x24, 0x40);
  TestRegAddrBase(cmp, r12, r13, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x65,
                  0x40);
  TestRegAddrBase(cmp, r13, r14, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x6E,
                  0x40);
  TestRegAddrBase(cmp, r14, r15, 0x40, i16, 6, 0x66, 0x67, 0x45, 0x3B, 0x77,
                  0x40);
  TestRegAddrBase(cmp, r15, eax, 0x40, i16, 6, 0x66, 0x67, 0x44, 0x3B, 0x78,
                  0x40);

  TestRegAddrBase(cmp, eax, ecx, 0x40, i8, 4, 0x67, 0x3A, 0x41, 0x40);
  TestRegAddrBase(cmp, ecx, edx, 0x40, i8, 4, 0x67, 0x3A, 0x4A, 0x40);
  TestRegAddrBase(cmp, edx, ebx, 0x40, i8, 4, 0x67, 0x3A, 0x53, 0x40);
  TestRegAddrBase(cmp, ebx, esp, 0x40, i8, 5, 0x67, 0x3A, 0x5C, 0x24, 0x40);
  TestRegAddrBase(cmp, esp, ebp, 0x40, i8, 4, 0x67, 0x3A, 0x65, 0x40);
  TestRegAddrBase(cmp, ebp, esi, 0x40, i8, 5, 0x67, 0x40, 0x3A, 0x6E, 0x40);
  TestRegAddrBase(cmp, esi, edi, 0x40, i8, 5, 0x67, 0x40, 0x3A, 0x77, 0x40);
  TestRegAddrBase(cmp, edi, r8, 0x40, i8, 5, 0x67, 0x41, 0x3A, 0x78, 0x40);
  TestRegAddrBase(cmp, r8, r9, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x41, 0x40);
  TestRegAddrBase(cmp, r9, r10, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x4A, 0x40);
  TestRegAddrBase(cmp, r10, r11, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x53, 0x40);
  TestRegAddrBase(cmp, r11, r12, 0x40, i8, 6, 0x67, 0x45, 0x3A, 0x5C, 0x24,
                  0x40);
  TestRegAddrBase(cmp, r12, r13, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x65, 0x40);
  TestRegAddrBase(cmp, r13, r14, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x6E, 0x40);
  TestRegAddrBase(cmp, r14, r15, 0x40, i8, 5, 0x67, 0x45, 0x3A, 0x77, 0x40);
  TestRegAddrBase(cmp, r15, eax, 0x40, i8, 5, 0x67, 0x44, 0x3A, 0x78, 0x40);

  /* cmp GPR, Imm32(Base) */
  TestRegAddrBase(cmp, eax, ecx, 0xF0, i32, 7, 0x67, 0x3B, 0x81, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ecx, edx, 0xF0, i32, 7, 0x67, 0x3B, 0x8A, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, edx, ebx, 0xF0, i32, 7, 0x67, 0x3B, 0x93, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ebx, esp, 0xF0, i32, 8, 0x67, 0x3B, 0x9C, 0x24, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, esp, ebp, 0xF0, i32, 7, 0x67, 0x3B, 0xA5, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0xF0, i32, 7, 0x67, 0x3B, 0xAE, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, esi, edi, 0xF0, i32, 7, 0x67, 0x3B, 0xB7, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, edi, r8, 0xF0, i32, 8, 0x67, 0x41, 0x3B, 0xB8, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r8, r9, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0x81, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, r9, r10, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0x8A, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r10, r11, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0x93, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r11, r12, 0xF0, i32, 9, 0x67, 0x45, 0x3B, 0x9C, 0x24,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r12, r13, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0xA5, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0xAE, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r14, r15, 0xF0, i32, 8, 0x67, 0x45, 0x3B, 0xB7, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r15, eax, 0xF0, i32, 8, 0x67, 0x44, 0x3B, 0xB8, 0xF0,
                  0x00, 0x00, 0x00);

  TestRegAddrBase(cmp, eax, ecx, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0x81, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, ecx, edx, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0x8A, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, edx, ebx, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0x93, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, ebx, esp, 0xF0, i16, 9, 0x66, 0x67, 0x3B, 0x9C, 0x24,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, esp, ebp, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0xa5, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0xaE, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, esi, edi, 0xF0, i16, 8, 0x66, 0x67, 0x3B, 0xb7, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, edi, r8, 0xF0, i16, 9, 0x66, 0x67, 0x41, 0x3B, 0xb8,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r8, r9, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0x81, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r9, r10, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0x8A,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r10, r11, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0x93,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r11, r12, 0xF0, i16, 10, 0x66, 0x67, 0x45, 0x3B, 0x9C,
                  0x24, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r12, r13, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0xa5,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0xaE,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r14, r15, 0xF0, i16, 9, 0x66, 0x67, 0x45, 0x3B, 0xb7,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r15, eax, 0xF0, i16, 9, 0x66, 0x67, 0x44, 0x3B, 0xb8,
                  0xF0, 0x00, 0x00, 0x00);

  TestRegAddrBase(cmp, eax, ecx, 0xF0, i8, 7, 0x67, 0x3A, 0x81, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ecx, edx, 0xF0, i8, 7, 0x67, 0x3A, 0x8A, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, edx, ebx, 0xF0, i8, 7, 0x67, 0x3A, 0x93, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ebx, esp, 0xF0, i8, 8, 0x67, 0x3A, 0x9C, 0x24, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, esp, ebp, 0xF0, i8, 7, 0x67, 0x3A, 0xA5, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, ebp, esi, 0xF0, i8, 8, 0x67, 0x40, 0x3A, 0xAE, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, esi, edi, 0xF0, i8, 8, 0x67, 0x40, 0x3A, 0xB7, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, edi, r8, 0xF0, i8, 8, 0x67, 0x41, 0x3A, 0xB8, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, r8, r9, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0x81, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, r9, r10, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0x8A, 0xF0, 0x00,
                  0x00, 0x00);
  TestRegAddrBase(cmp, r10, r11, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0x93, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r11, r12, 0xF0, i8, 9, 0x67, 0x45, 0x3A, 0x9C, 0x24,
                  0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r12, r13, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0xA5, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r13, r14, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0xAE, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r14, r15, 0xF0, i8, 8, 0x67, 0x45, 0x3A, 0xB7, 0xF0,
                  0x00, 0x00, 0x00);
  TestRegAddrBase(cmp, r15, eax, 0xF0, i8, 8, 0x67, 0x44, 0x3A, 0xB8, 0xF0,
                  0x00, 0x00, 0x00);

  /* cmp GPR, Imm(,Index,Scale) */
  TestRegAddrScaledIndex(cmp, eax, ecx, 1, 0, i32, 8, 0x67, 0x3B, 0x04, 0x0D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ecx, edx, 2, 0, i32, 8, 0x67, 0x3B, 0x0C, 0x55,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edx, ebx, 4, 0, i32, 8, 0x67, 0x3B, 0x14, 0x9D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r8, r9, 1, 0, i32, 9, 0x67, 0x46, 0x3B, 0x04,
                         0x0D, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r9, r10, 2, 0, i32, 9, 0x67, 0x46, 0x3B, 0x0C,
                         0x55, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r10, r11, 4, 0, i32, 9, 0x67, 0x46, 0x3B, 0x14,
                         0x9D, 0x00, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrScaledIndex(cmp, esp, ebp, 8, 0, i32, 8, 0x67, 0x3B, 0x24, 0xED,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebp, esi, 1, 0, i32, 8, 0x67, 0x3B, 0x2C, 0x35,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, esi, edi, 2, 0, i32, 8, 0x67, 0x3B, 0x34, 0x7D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edi, eax, 4, 0, i32, 8, 0x67, 0x3B, 0x3C, 0x85,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebx, ecx, 8, 0, i32, 8, 0x67, 0x3B, 0x1C, 0xCD,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r12, r13, 8, 0, i32, 9, 0x67, 0x46, 0x3B, 0x24,
                         0xED, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r13, r14, 1, 0, i32, 9, 0x67, 0x46, 0x3B, 0x2C,
                         0x35, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r14, r15, 2, 0, i32, 9, 0x67, 0x46, 0x3B, 0x34,
                         0x7D, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r15, r8, 4, 0, i32, 9, 0x67, 0x46, 0x3B, 0x3C,
                         0x85, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r11, r9, 8, 0, i32, 9, 0x67, 0x46, 0x3B, 0x1C,
                         0xCD, 0x00, 0x00, 0x00, 0x00);

  TestRegAddrScaledIndex(cmp, eax, ecx, 8, 0, i16, 9, 0x66, 0x67, 0x3B, 0x04,
                         0xCD, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ecx, edx, 1, 0, i16, 9, 0x66, 0x67, 0x3B, 0x0C,
                         0x15, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edx, ebx, 2, 0, i16, 9, 0x66, 0x67, 0x3B, 0x14,
                         0x5D, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r8, r9, 8, 0, i16, 10, 0x66, 0x67, 0x46, 0x3B,
                         0x04, 0xCD, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r9, r10, 1, 0, i16, 10, 0x66, 0x67, 0x46, 0x3B,
                         0x0C, 0x15, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r10, r11, 2, 0, i16, 10, 0x66, 0x67, 0x46, 0x3B,
                         0x14, 0x5D, 0x00, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrScaledIndex(cmp, esp, ebp, 4, 0, i16, 9, 0x66, 0x67, 0x3B, 0x24,
                         0xAD, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebp, esi, 8, 0, i16, 9, 0x66, 0x67, 0x3B, 0x2C,
                         0xF5, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, esi, edi, 1, 0, i16, 9, 0x66, 0x67, 0x3B, 0x34,
                         0x3D, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edi, eax, 2, 0, i16, 9, 0x66, 0x67, 0x3B, 0x3C,
                         0x45, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebx, ecx, 8, 0, i16, 9, 0x66, 0x67, 0x3B, 0x1C,
                         0xCD, 0x00, 0x00, 0x00, 0x00);

  TestRegAddrScaledIndex(cmp, eax, ecx, 4, 0, i8, 8, 0x67, 0x3A, 0x04, 0x8D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ecx, edx, 8, 0, i8, 8, 0x67, 0x3A, 0x0C, 0xD5,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edx, ebx, 1, 0, i8, 8, 0x67, 0x3A, 0x14, 0x1D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r8, r9, 4, 0, i8, 9, 0x67, 0x46, 0x3A, 0x04, 0x8D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r9, r10, 8, 0, i8, 9, 0x67, 0x46, 0x3A, 0x0C,
                         0xD5, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r10, r11, 1, 0, i8, 9, 0x67, 0x46, 0x3A, 0x14,
                         0x1D, 0x00, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrScaledIndex(cmp, esp, ebp, 2, 0, i8, 8, 0x67, 0x3A, 0x24, 0x6D,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebp, esi, 4, 0, i8, 9, 0x67, 0x40, 0x3A, 0x2C,
                         0xB5, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, esi, edi, 8, 0, i8, 9, 0x67, 0x40, 0x3A, 0x34,
                         0xFD, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, edi, eax, 1, 0, i8, 9, 0x67, 0x40, 0x3A, 0x3C,
                         0x05, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, ebx, ecx, 8, 0, i8, 8, 0x67, 0x3a, 0x1C, 0xCD,
                         0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r12, r13, 2, 0, i8, 9, 0x67, 0x46, 0x3A, 0x24,
                         0x6D, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r13, r14, 4, 0, i8, 9, 0x67, 0x46, 0x3A, 0x2C,
                         0xB5, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r14, r15, 8, 0, i8, 9, 0x67, 0x46, 0x3A, 0x34,
                         0xFD, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r15, r8, 1, 0, i8, 9, 0x67, 0x46, 0x3A, 0x3C,
                         0x05, 0x00, 0x00, 0x00, 0x00);
  TestRegAddrScaledIndex(cmp, r11, r9, 8, 0, i8, 9, 0x67, 0x46, 0x3a, 0x1C,
                         0xCD, 0x00, 0x00, 0x00, 0x00);

  /* cmp GPR, 0(Base,Index,Scale) */
  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0, i32, 4, 0x67, 0x3B, 0x04,
                             0x11);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0, i32, 4, 0x67, 0x3B, 0x0C,
                             0x5A);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x04, 0x11);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x0C, 0x5A);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0, i32, 4, 0x67, 0x3B, 0x1C,
                             0xAC);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0, i32, 5, 0x67, 0x3B, 0x64,
                             0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0, i32, 4, 0x67, 0x3B, 0x2C,
                             0x3E);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0, i32, 4, 0x67, 0x3B, 0x34,
                             0x47);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0, i32, 4, 0x67, 0x3B, 0x3C,
                             0x98);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0, i32, 4, 0x67, 0x3B, 0x1C,
                             0xD1);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x1C, 0xAC);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0, i32, 6, 0x67, 0x47, 0x3B,
                             0x64, 0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x2C, 0x3E);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x34, 0x47);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x3C, 0x98);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0, i32, 5, 0x67, 0x47, 0x3B,
                             0x1C, 0xD1);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x04, 0x11);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x0C, 0x5A);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x04, 0x11);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x0C, 0x5A);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x1C, 0xAC);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0, i16, 6, 0x66, 0x67, 0x3B,
                             0x64, 0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x2C, 0x3E);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x34, 0x47);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x3C, 0x98);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0, i16, 5, 0x66, 0x67, 0x3B,
                             0x1C, 0xD1);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x1C, 0xAC);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0, i16, 7, 0x66, 0x67, 0x47,
                             0x3B, 0x64, 0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x2C, 0x3E);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x34, 0x47);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x3C, 0x98);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0, i16, 6, 0x66, 0x67, 0x47,
                             0x3B, 0x1C, 0xD1);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0, i8, 4, 0x67, 0x3A, 0x04,
                             0x11);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0, i8, 4, 0x67, 0x3A, 0x0C,
                             0x5A);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x04, 0x11);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x0C, 0x5A);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0, i8, 4, 0x67, 0x3A, 0x1C,
                             0xAC);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0, i8, 5, 0x67, 0x3A, 0x64,
                             0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0, i8, 5, 0x67, 0x40, 0x3A,
                             0x2C, 0x3E);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0, i8, 5, 0x67, 0x40, 0x3A,
                             0x34, 0x47);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0, i8, 5, 0x67, 0x40, 0x3A,
                             0x3C, 0x98);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0, i8, 4, 0x67, 0x3A, 0x1C,
                             0xD1);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x1C, 0xAC);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0, i8, 6, 0x67, 0x47, 0x3A,
                             0x64, 0xF5, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x2C, 0x3E);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x34, 0x47);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x3C, 0x98);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0, i8, 5, 0x67, 0x47, 0x3A,
                             0x1C, 0xD1);

  /* cmp GPR, Imm8(Base,Index,Scale) */
  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0x40, i32, 5, 0x67, 0x3B,
                             0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0x40, i32, 5, 0x67, 0x3B,
                             0x4C, 0x5A, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x4C, 0x5A, 0x40);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0x40, i32, 5, 0x67, 0x3B,
                             0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0x40, i32, 5, 0x67, 0x3B,
                             0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0x40, i32, 5, 0x67, 0x3B,
                             0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0x40, i32, 5, 0x67, 0x3B,
                             0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0x40, i32, 5, 0x67, 0x3B,
                             0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0x40, i32, 5, 0x67, 0x3B,
                             0x5C, 0xD1, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0x40, i32, 6, 0x67, 0x47,
                             0x3B, 0x5C, 0xD1, 0x40);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x4C, 0x5A, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x4C, 0x5A, 0x40);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0x40, i16, 6, 0x66, 0x67,
                             0x3B, 0x5C, 0xD1, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0x40, i16, 7, 0x66, 0x67,
                             0x47, 0x3B, 0x5C, 0xD1, 0x40);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0x40, i8, 5, 0x67, 0x3A,
                             0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0x40, i8, 5, 0x67, 0x3A,
                             0x4C, 0x5A, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0x40, i8, 6, 0x67, 0x47, 0x3A,
                             0x44, 0x11, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x4C, 0x5A, 0x40);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0x40, i8, 5, 0x67, 0x3A,
                             0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0x40, i8, 5, 0x67, 0x3A,
                             0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0x40, i8, 6, 0x67, 0x40,
                             0x3A, 0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0x40, i8, 6, 0x67, 0x40,
                             0x3A, 0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0x40, i8, 6, 0x67, 0x40,
                             0x3A, 0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0x40, i8, 5, 0x67, 0x3A,
                             0x5C, 0xD1, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x5C, 0xAC, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x64, 0xF5, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x6C, 0x3E, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x74, 0x47, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x7C, 0x98, 0x40);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0x40, i8, 6, 0x67, 0x47,
                             0x3A, 0x5C, 0xD1, 0x40);

  /* cmp GPR, Imm32(Base,Index,Scale) */
  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0xF0, i32, 8, 0x67, 0x3B,
                             0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0xF0, i32, 8, 0x67, 0x3B,
                             0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0xF0, i32, 8, 0x67, 0x3B,
                             0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0xF0, i32, 8, 0x67, 0x3B,
                             0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0xF0, i32, 8, 0x67, 0x3B,
                             0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0xF0, i32, 8, 0x67, 0x3B,
                             0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0xF0, i32, 8, 0x67, 0x3B,
                             0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0xF0, i32, 8, 0x67, 0x3B,
                             0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0xF0, i32, 9, 0x67, 0x47,
                             0x3B, 0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0xF0, i16, 9, 0x66, 0x67,
                             0x3B, 0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0xF0, i16, 10, 0x66, 0x67,
                             0x47, 0x3B, 0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);

  TestRegAddrBaseScaledIndex(cmp, eax, ecx, edx, 1, 0xF0, i8, 8, 0x67, 0x3A,
                             0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ecx, edx, ebx, 2, 0xF0, i8, 8, 0x67, 0x3A,
                             0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r8, r9, r10, 1, 0xF0, i8, 9, 0x67, 0x47, 0x3A,
                             0x84, 0x11, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r9, r10, r11, 2, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0x8C, 0x5A, 0xF0, 0x00, 0x00, 0x00);
  // esp cannot be an scaled index.
  TestRegAddrBaseScaledIndex(cmp, ebx, esp, ebp, 4, 0xF0, i8, 8, 0x67, 0x3A,
                             0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esp, ebp, esi, 8, 0xF0, i8, 8, 0x67, 0x3A,
                             0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebp, esi, edi, 1, 0xF0, i8, 9, 0x67, 0x40,
                             0x3A, 0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, esi, edi, eax, 2, 0xF0, i8, 9, 0x67, 0x40,
                             0x3A, 0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, edi, eax, ebx, 4, 0xF0, i8, 9, 0x67, 0x40,
                             0x3A, 0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, ebx, ecx, edx, 8, 0xF0, i8, 8, 0x67, 0x3A,
                             0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r12, r13, 4, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0x9C, 0xAC, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r12, r13, r14, 8, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0xA4, 0xF5, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r13, r14, r15, 1, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0xAC, 0x3E, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r14, r15, r8, 2, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0xB4, 0x47, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r15, r8, r11, 4, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0xBC, 0x98, 0xF0, 0x00, 0x00, 0x00);
  TestRegAddrBaseScaledIndex(cmp, r11, r9, r10, 8, 0xF0, i8, 9, 0x67, 0x47,
                             0x3A, 0x9C, 0xD1, 0xF0, 0x00, 0x00, 0x00);

  /* cmp Addr, Imm */
  // Note: at this point we trust the assembler knows how to encode addresses,
  // so no more exhaustive addressing mode testing.
  TestAddrBaseScaledIndexImm(cmp, eax, ecx, 1, 0xF0, 0x12, i32, 9, 0x67, 0x83,
                             0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x12);
  TestAddrBaseScaledIndexImm(cmp, ecx, edx, 1, 0xF0, 0xF0, i32, 12, 0x67, 0x81,
                             0xBC, 0x11, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00,
                             0x00, 0x00);
  TestAddrBaseScaledIndexImm(cmp, r8, r9, 1, 0xF0, 0x12, i32, 10, 0x67, 0x43,
                             0x83, 0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x12);
  TestAddrBaseScaledIndexImm(cmp, r9, r10, 1, 0xF0, 0xF0, i32, 13, 0x67, 0x43,
                             0x81, 0xBC, 0x11, 0xF0, 0x00, 0x00, 0x00, 0xF0,
                             0x00, 0x00, 0x00);

  TestAddrBaseScaledIndexImm(cmp, eax, ecx, 1, 0xF0, 0x12, i16, 10, 0x66, 0x67,
                             0x83, 0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x12);
  TestAddrBaseScaledIndexImm(cmp, ecx, edx, 1, 0xF0, 0xF0, i16, 11, 0x66, 0x67,
                             0x81, 0xBC, 0x11, 0xF0, 0x00, 0x00, 0x00, 0xF0,
                             0x00);
  TestAddrBaseScaledIndexImm(cmp, r8, r9, 1, 0xF0, 0x12, i16, 11, 0x66, 0x67,
                             0x43, 0x83, 0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00,
                             0x12);
  TestAddrBaseScaledIndexImm(cmp, r9, r10, 1, 0xF0, 0xF0, i16, 12, 0x66, 0x67,
                             0x43, 0x81, 0xBC, 0x11, 0xF0, 0x00, 0x00, 0x00,
                             0xF0, 0x00);

  TestAddrBaseScaledIndexImm(cmp, eax, ecx, 1, 0xF0, 0x12, i8, 9, 0x67, 0x80,
                             0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x12);
  TestAddrBaseScaledIndexImm(cmp, r8, r9, 1, 0xF0, 0x12, i8, 10, 0x67, 0x43,
                             0x80, 0xBC, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x12);

  /* cmp Addr, GPR */
  TestAddrBaseScaledIndexReg(cmp, eax, ecx, 1, 0xF0, edx, i32, 8, 0x67, 0x39,
                             0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);
  TestAddrBaseScaledIndexReg(cmp, r8, r9, 1, 0xF0, r10, i32, 9, 0x67, 0x47,
                             0x39, 0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);

  TestAddrBaseScaledIndexReg(cmp, eax, ecx, 1, 0xF0, edx, i16, 9, 0x66, 0x67,
                             0x39, 0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);
  TestAddrBaseScaledIndexReg(cmp, r8, r9, 1, 0xF0, r10, i16, 10, 0x66, 0x67,
                             0x47, 0x39, 0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);

  TestAddrBaseScaledIndexReg(cmp, eax, ecx, 1, 0xF0, edx, i8, 8, 0x67, 0x38,
                             0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);
  TestAddrBaseScaledIndexReg(cmp, r8, r9, 1, 0xF0, r10, i8, 9, 0x67, 0x47, 0x38,
                             0x94, 0x08, 0xF0, 0x00, 0x00, 0x00);

#undef TestAddrBaseScaledIndexReg
#undef TestAddrBaseScaledIndexImm
#undef TestRegAddrBaseScaledIndex
#undef TestRegAddrScaledIndex
#undef TestRegAddrBase
#undef TestRegAbsoluteAddr
#undef TestRegImm
#undef TestRegReg
}

TEST_F(AssemblerX8664Test, ScratchpadGettersAndSetters) {
  const uint32_t S0 = allocateDword();
  const uint32_t S1 = allocateDword();
  const uint32_t S2 = allocateDword();
  const uint32_t S3 = allocateDword();
  AssembledTest test = assemble();
  test.setDwordTo(S0, 0xBEEF0000u);
  test.setDwordTo(S1, 0xDEADu);
  test.setDwordTo(S2, 0x20406080u);
  ASSERT_EQ(0xBEEF0000u, test.contentsOfDword(S0));
  ASSERT_EQ(0xDEADu, test.contentsOfDword(S1));
  ASSERT_EQ(0x20406080u, test.contentsOfDword(S2));
  ASSERT_EQ(0xDEADBEEF0000ull, test.contentsOfQword(S0));
  ASSERT_EQ(0x204060800000DEADull, test.contentsOfQword(S1));

  test.setQwordTo(S1, 0x1234567890ABCDEFull);
  ASSERT_EQ(0x1234567890ABCDEFull, test.contentsOfQword(S1));
  test.setDwordTo(S0, 0xBEEF0000u);
  ASSERT_EQ(0x90ABCDEFull, test.contentsOfDword(S1));
  ASSERT_EQ(0x12345678ull, test.contentsOfDword(S2));

  test.setDwordTo(S0, 1.0f);
  ASSERT_FLOAT_EQ(1.0f, test.contentsOfDword<float>(S0));
  test.setQwordTo(S0, 3.14);
  ASSERT_DOUBLE_EQ(3.14, test.contentsOfQword<double>(S0));

  test.setDqwordTo(S0, Dqword(1.0f, 2.0f, 3.0f, 4.0f));
  ASSERT_EQ(Dqword(1.0f, 2.0f, 3.0f, 4.0f), test.contentsOfDqword(S0));
  EXPECT_FLOAT_EQ(1.0f, test.contentsOfDword<float>(S0));
  EXPECT_FLOAT_EQ(2.0f, test.contentsOfDword<float>(S1));
  EXPECT_FLOAT_EQ(3.0f, test.contentsOfDword<float>(S2));
  EXPECT_FLOAT_EQ(4.0f, test.contentsOfDword<float>(S3));
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
