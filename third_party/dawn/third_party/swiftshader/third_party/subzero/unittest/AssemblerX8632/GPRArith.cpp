//===- subzero/unittest/AssemblerX8632/GPRArith.cpp -----------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AssemblerX8632/TestUtil.h"

namespace Ice {
namespace X8632 {
namespace Test {
namespace {

TEST_F(AssemblerX8632LowLevelTest, PushalPopal) {
  // These are invalid in x86-64, so we can't write tests which will execute
  // these instructions.
  __ pushal();
  __ popal();

  constexpr size_t ByteCount = 2;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Pushal = 0x60;
  constexpr uint8_t Popal = 0x61;

  verifyBytes<ByteCount>(codeBytes(), Pushal, Popal);
}

TEST_F(AssemblerX8632Test, PopAddr) {
  const uint32_t T0 = allocateDword();
  constexpr uint32_t V0 = 0xEFAB;

  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0xC0FFEE));
  __ pushl(GPRRegister::Encoded_Reg_eax);
  __ popl(dwordAddress(T0));

  AssembledTest test = assemble();
  test.setDwordTo(T0, V0);

  test.run();

  ASSERT_EQ(0xC0FFEEul, test.contentsOfDword(T0));
}

TEST_F(AssemblerX8632Test, SetCC) {
#define TestSetCC(C, Src0, Value0, Src1, Value1, Dest, IsTrue)                 \
  do {                                                                         \
    const uint32_t T0 = allocateDword();                                       \
    constexpr uint32_t V0 = 0xF00F00;                                          \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src0, Immediate(Value0));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src1, Immediate(Value1));   \
    __ cmp(IceType_i32, GPRRegister::Encoded_Reg_##Src0,                       \
           GPRRegister::Encoded_Reg_##Src1);                                   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dest, Immediate(0));        \
    __ setcc(Cond::Br_##C, ByteRegister(GPRRegister::Encoded_Reg_##Dest));     \
    __ setcc(Cond::Br_##C, dwordAddress(T0));                                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
                                                                               \
    test.run();                                                                \
                                                                               \
    EXPECT_EQ(IsTrue, test.Dest())                                             \
        << "(" #C ", " #Src0 ", " #Value0 ", " #Src1 ", " #Value1 ", " #Dest   \
           ", " #IsTrue ")";                                                   \
    EXPECT_EQ((0xF00F00 | IsTrue), test.contentsOfDword(T0))                   \
        << "(" #C ", " #Src0 ", " #Value0 ", " #Src1 ", " #Value1 ", " #Dest   \
           ", " #IsTrue ")";                                                   \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestSetCC(o, eax, 0x80000000u, ebx, 0x1u, ecx, 1u);
  TestSetCC(o, eax, 0x1u, ebx, 0x10000000u, ecx, 0u);

  TestSetCC(no, ebx, 0x1u, ecx, 0x10000000u, edx, 1u);
  TestSetCC(no, ebx, 0x80000000u, ecx, 0x1u, edx, 0u);

  TestSetCC(b, ecx, 0x1, edx, 0x80000000u, eax, 1u);
  TestSetCC(b, ecx, 0x80000000u, edx, 0x1u, eax, 0u);

  TestSetCC(ae, edx, 0x80000000u, edi, 0x1u, ebx, 1u);
  TestSetCC(ae, edx, 0x1u, edi, 0x80000000u, ebx, 0u);

  TestSetCC(e, edi, 0x1u, esi, 0x1u, ecx, 1u);
  TestSetCC(e, edi, 0x1u, esi, 0x11111u, ecx, 0u);

  TestSetCC(ne, esi, 0x80000000u, eax, 0x1u, edx, 1u);
  TestSetCC(ne, esi, 0x1u, eax, 0x1u, edx, 0u);

  TestSetCC(be, eax, 0x1u, ebx, 0x80000000u, eax, 1u);
  TestSetCC(be, eax, 0x80000000u, ebx, 0x1u, eax, 0u);

  TestSetCC(a, ebx, 0x80000000u, ecx, 0x1u, ebx, 1u);
  TestSetCC(a, ebx, 0x1u, ecx, 0x80000000u, ebx, 0u);

  TestSetCC(s, ecx, 0x1u, edx, 0x80000000u, ecx, 1u);
  TestSetCC(s, ecx, 0x80000000u, edx, 0x1u, ecx, 0u);

  TestSetCC(ns, edx, 0x80000000u, edi, 0x1u, ecx, 1u);
  TestSetCC(ns, edx, 0x1u, edi, 0x80000000u, ecx, 0u);

  TestSetCC(p, edi, 0x80000000u, esi, 0x1u, edx, 1u);
  TestSetCC(p, edi, 0x1u, esi, 0x80000000u, edx, 0u);

  TestSetCC(np, esi, 0x1u, edi, 0x80000000u, eax, 1u);
  TestSetCC(np, esi, 0x80000000u, edi, 0x1u, eax, 0u);

  TestSetCC(l, edi, 0x80000000u, eax, 0x1u, ebx, 1u);
  TestSetCC(l, edi, 0x1u, eax, 0x80000000u, ebx, 0u);

  TestSetCC(ge, eax, 0x1u, ebx, 0x80000000u, ecx, 1u);
  TestSetCC(ge, eax, 0x80000000u, ebx, 0x1u, ecx, 0u);

  TestSetCC(le, ebx, 0x80000000u, ecx, 0x1u, edx, 1u);
  TestSetCC(le, ebx, 0x1u, ecx, 0x80000000u, edx, 0u);

#undef TestSetCC
}

TEST_F(AssemblerX8632Test, Lea) {
#define TestLeaBaseDisp(Base, BaseValue, Disp, Dst)                            \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Base ", " #BaseValue ", " #Dst ")";                               \
    if (GPRRegister::Encoded_Reg_##Base != GPRRegister::Encoded_Reg_esp &&     \
        GPRRegister::Encoded_Reg_##Base != GPRRegister::Encoded_Reg_ebp) {     \
      __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Base,                     \
             Immediate(BaseValue));                                            \
    }                                                                          \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Address(GPRRegister::Encoded_Reg_##Base, Disp,                      \
                   AssemblerFixup::NoFixup));                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ(test.Base() + (Disp), test.Dst())                                \
        << TestString << " with Disp " << Disp;                                \
    reset();                                                                   \
  } while (0)

#define TestLeaIndex32bitDisp(Index, IndexValue, Disp, Dst0, Dst1, Dst2, Dst3) \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Index ", " #IndexValue ", " #Dst0 ", " #Dst1 ", " #Dst2           \
        ", " #Dst3 ")";                                                        \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Index,                      \
           Immediate(IndexValue));                                             \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst0,                       \
           Address(GPRRegister::Encoded_Reg_##Index, Traits::TIMES_1, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst1,                       \
           Address(GPRRegister::Encoded_Reg_##Index, Traits::TIMES_2, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst2,                       \
           Address(GPRRegister::Encoded_Reg_##Index, Traits::TIMES_4, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst3,                       \
           Address(GPRRegister::Encoded_Reg_##Index, Traits::TIMES_8, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((test.Index() << Traits::TIMES_1) + (Disp), test.Dst0())         \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index() << Traits::TIMES_2) + (Disp), test.Dst1())         \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index() << Traits::TIMES_4) + (Disp), test.Dst2())         \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index() << Traits::TIMES_8) + (Disp), test.Dst3())         \
        << TestString << " " << Disp;                                          \
    reset();                                                                   \
  } while (0)

#define TestLeaBaseIndexDisp(Base, BaseValue, Index, IndexValue, Disp, Dst0,   \
                             Dst1, Dst2, Dst3)                                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Base ", " #BaseValue ", " #Index ", " #IndexValue ", " #Dst0      \
        ", " #Dst1 ", " #Dst2 ", " #Dst3 ")";                                  \
    if (GPRRegister::Encoded_Reg_##Base != GPRRegister::Encoded_Reg_esp &&     \
        GPRRegister::Encoded_Reg_##Base != GPRRegister::Encoded_Reg_ebp) {     \
      __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Base,                     \
             Immediate(BaseValue));                                            \
    }                                                                          \
    /* esp is not a valid index register. */                                   \
    if (GPRRegister::Encoded_Reg_##Index != GPRRegister::Encoded_Reg_ebp) {    \
      __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Index,                    \
             Immediate(IndexValue));                                           \
    }                                                                          \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst0,                       \
           Address(GPRRegister::Encoded_Reg_##Base,                            \
                   GPRRegister::Encoded_Reg_##Index, Traits::TIMES_1, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst1,                       \
           Address(GPRRegister::Encoded_Reg_##Base,                            \
                   GPRRegister::Encoded_Reg_##Index, Traits::TIMES_2, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst2,                       \
           Address(GPRRegister::Encoded_Reg_##Base,                            \
                   GPRRegister::Encoded_Reg_##Index, Traits::TIMES_4, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst3,                       \
           Address(GPRRegister::Encoded_Reg_##Base,                            \
                   GPRRegister::Encoded_Reg_##Index, Traits::TIMES_8, Disp,    \
                   AssemblerFixup::NoFixup));                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    uint32_t ExpectedIndexValue = test.Index();                                \
    if (GPRRegister::Encoded_Reg_##Index == GPRRegister::Encoded_Reg_esp) {    \
      ExpectedIndexValue = 0;                                                  \
    }                                                                          \
    ASSERT_EQ(test.Base() + (ExpectedIndexValue << Traits::TIMES_1) + (Disp),  \
              test.Dst0())                                                     \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base() + (ExpectedIndexValue << Traits::TIMES_2) + (Disp),  \
              test.Dst1())                                                     \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base() + (ExpectedIndexValue << Traits::TIMES_4) + (Disp),  \
              test.Dst2())                                                     \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base() + (ExpectedIndexValue << Traits::TIMES_8) + (Disp),  \
              test.Dst3())                                                     \
        << TestString << " " << Disp;                                          \
    reset();                                                                   \
  } while (0)

  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaBaseDisp(eax, 0x10000Fu, Disp, ebx);
    TestLeaBaseDisp(ebx, 0x20000Fu, Disp, ecx);
    TestLeaBaseDisp(ecx, 0x30000Fu, Disp, edx);
    TestLeaBaseDisp(edx, 0x40000Fu, Disp, esi);
    TestLeaBaseDisp(esi, 0x50000Fu, Disp, edi);
    TestLeaBaseDisp(edi, 0x60000Fu, Disp, eax);
    TestLeaBaseDisp(esp, 0x11000Fu, Disp, eax);
    TestLeaBaseDisp(ebp, 0x22000Fu, Disp, ecx);
  }

  // esp is not a valid index register.
  // ebp is not valid in this addressing mode (rm = 0).
  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaIndex32bitDisp(eax, 0x2000u, Disp, ebx, ecx, edx, esi);
    TestLeaIndex32bitDisp(ebx, 0x4000u, Disp, ecx, edx, esi, edi);
    TestLeaIndex32bitDisp(ecx, 0x6000u, Disp, edx, esi, edi, eax);
    TestLeaIndex32bitDisp(edx, 0x8000u, Disp, esi, edi, eax, ebx);
    TestLeaIndex32bitDisp(esi, 0xA000u, Disp, edi, eax, ebx, ecx);
    TestLeaIndex32bitDisp(edi, 0xC000u, Disp, eax, ebx, ecx, edx);
  }

  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaBaseIndexDisp(eax, 0x100000u, ebx, 0x600u, Disp, ecx, edx, esi, edi);
    TestLeaBaseIndexDisp(ebx, 0x200000u, ecx, 0x500u, Disp, edx, esi, edi, eax);
    TestLeaBaseIndexDisp(ecx, 0x300000u, edx, 0x400u, Disp, esi, edi, eax, ebx);
    TestLeaBaseIndexDisp(edx, 0x400000u, esi, 0x300u, Disp, edi, eax, ebx, ecx);
    TestLeaBaseIndexDisp(esi, 0x500000u, edi, 0x200u, Disp, eax, ebx, ecx, edx);
    TestLeaBaseIndexDisp(edi, 0x600000u, eax, 0x100u, Disp, ebx, ecx, edx, esi);

    /* Initializers are ignored when Src[01] is ebp/esp. */
    TestLeaBaseIndexDisp(esp, 0, ebx, 0x6000u, Disp, ecx, edx, esi, edi);
    TestLeaBaseIndexDisp(esp, 0, ecx, 0x5000u, Disp, edx, esi, edi, eax);
    TestLeaBaseIndexDisp(esp, 0, edx, 0x4000u, Disp, esi, edi, eax, ebx);
    TestLeaBaseIndexDisp(esp, 0, esi, 0x3000u, Disp, edi, eax, ebx, ecx);
    TestLeaBaseIndexDisp(esp, 0, edi, 0x2000u, Disp, eax, ebx, ecx, edx);
    TestLeaBaseIndexDisp(esp, 0, eax, 0x1000u, Disp, ebx, ecx, edx, esi);

    TestLeaBaseIndexDisp(ebp, 0, ebx, 0x6000u, Disp, ecx, edx, esi, edi);
    TestLeaBaseIndexDisp(ebp, 0, ecx, 0x5000u, Disp, edx, esi, edi, eax);
    TestLeaBaseIndexDisp(ebp, 0, edx, 0x4000u, Disp, esi, edi, eax, ebx);
    TestLeaBaseIndexDisp(ebp, 0, esi, 0x3000u, Disp, edi, eax, ebx, ecx);
    TestLeaBaseIndexDisp(ebp, 0, edi, 0x2000u, Disp, eax, ebx, ecx, edx);
    TestLeaBaseIndexDisp(ebp, 0, eax, 0x1000u, Disp, ebx, ecx, edx, esi);

    TestLeaBaseIndexDisp(eax, 0x1000000u, ebp, 0, Disp, ecx, edx, esi, edi);
    TestLeaBaseIndexDisp(ebx, 0x2000000u, ebp, 0, Disp, edx, esi, edi, eax);
    TestLeaBaseIndexDisp(ecx, 0x3000000u, ebp, 0, Disp, esi, edi, eax, ebx);
    TestLeaBaseIndexDisp(edx, 0x4000000u, ebp, 0, Disp, edi, eax, ebx, ecx);
    TestLeaBaseIndexDisp(esi, 0x5000000u, ebp, 0, Disp, eax, ebx, ecx, edx);
    TestLeaBaseIndexDisp(edi, 0x6000000u, ebp, 0, Disp, ebx, ecx, edx, esi);

    TestLeaBaseIndexDisp(esp, 0, ebp, 0, Disp, ebx, ecx, edx, esi);
  }

// Absolute addressing mode is tested in the Low Level tests. The encoding used
// by the assembler has different meanings in x86-32 and x86-64.
#undef TestLeaBaseIndexDisp
#undef TestLeaScaled32bitDisp
#undef TestLeaBaseDisp
}

TEST_F(AssemblerX8632LowLevelTest, LeaAbsolute) {
#define TestLeaAbsolute(Dst, Value)                                            \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Value ")";             \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Address(Value, AssemblerFixup::NoFixup));                           \
    static constexpr uint32_t ByteCount = 6;                                   \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    static constexpr uint8_t Opcode = 0x8D;                                    \
    static constexpr uint8_t ModRM =                                           \
        /*mod=*/0x00 | /*reg*/ (GPRRegister::Encoded_Reg_##Dst << 3) |         \
        /*rm*/ GPRRegister::Encoded_Reg_ebp;                                   \
    verifyBytes<ByteCount>(codeBytes(), Opcode, ModRM, (Value)&0xFF,           \
                           (Value >> 8) & 0xFF, (Value >> 16) & 0xFF,          \
                           (Value >> 24) & 0xFF);                              \
    reset();                                                                   \
  } while (0)

  TestLeaAbsolute(eax, 0x11BEEF22);
  TestLeaAbsolute(ebx, 0x33BEEF44);
  TestLeaAbsolute(ecx, 0x55BEEF66);
  TestLeaAbsolute(edx, 0x77BEEF88);
  TestLeaAbsolute(esi, 0x99BEEFAA);
  TestLeaAbsolute(edi, 0xBBBEEFBB);

#undef TesLeaAbsolute
}

TEST_F(AssemblerX8632Test, Test) {
  static constexpr uint32_t Mask8 = 0xFF;
  static constexpr uint32_t Mask16 = 0xFFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplRegReg(Dst, Value0, Src, Value1, Size)                         \
  do {                                                                         \
    static constexpr bool NearJump = true;                                     \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Size ")";           \
    static constexpr uint32_t ValueIfTrue = 0xBEEFFEEB;                        \
    static constexpr uint32_t ValueIfFalse = 0x11111111;                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(Value0));                                                 \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Value1));                                                 \
    __ test(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src);                                   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Immediate(ValueIfFalse));                                           \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Immediate(ValueIfTrue));                                            \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(((Value0)&Mask##Size) & ((Value1)&Mask##Size) ? ValueIfTrue      \
                                                            : ValueIfFalse,    \
              test.Dst())                                                      \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplRegImm(Dst, Value0, Imm, Size)                                 \
  do {                                                                         \
    static constexpr bool NearJump = true;                                     \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Imm ", " #Size ")";                        \
    static constexpr uint32_t ValueIfTrue = 0xBEEFFEEB;                        \
    static constexpr uint32_t ValueIfFalse = 0x11111111;                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(Value0));                                                 \
    __ test(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            Immediate((Imm)&Mask##Size));                                      \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Immediate(ValueIfFalse));                                           \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Immediate(ValueIfTrue));                                            \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(((Value0)&Mask##Size) & ((Imm)&Mask##Size) ? ValueIfTrue         \
                                                         : ValueIfFalse,       \
              test.Dst())                                                      \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplAddrReg(Value0, Src, Value1, Size)                             \
  do {                                                                         \
    static constexpr bool NearJump = true;                                     \
    static constexpr char TestString[] =                                       \
        "(Addr, " #Value0 ", " #Src ", " #Value1 ", " #Size ")";               \
    static constexpr uint32_t ValueIfTrue = 0xBEEFFEEB;                        \
    static constexpr uint32_t ValueIfFalse = 0x11111111;                       \
    const uint32_t T0 = allocateDword();                                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Value1));                                                 \
    __ test(IceType_i##Size, dwordAddress(T0),                                 \
            GPRRegister::Encoded_Reg_##Src);                                   \
    __ mov(IceType_i32, dwordAddress(T0), Immediate(ValueIfFalse));            \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, dwordAddress(T0), Immediate(ValueIfTrue));             \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, uint32_t(Value0));                                     \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(((Value0)&Mask##Size) & ((Value1)&Mask##Size) ? ValueIfTrue      \
                                                            : ValueIfFalse,    \
              test.contentsOfDword(T0))                                        \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplAddrImm(Value0, Value1, Size)                                  \
  do {                                                                         \
    static constexpr bool NearJump = true;                                     \
    static constexpr char TestString[] =                                       \
        "(Addr, " #Value0 ", " #Value1 ", " #Size ")";                         \
    static constexpr uint32_t ValueIfTrue = 0xBEEFFEEB;                        \
    static constexpr uint32_t ValueIfFalse = 0x11111111;                       \
    const uint32_t T0 = allocateDword();                                       \
                                                                               \
    __ test(IceType_i##Size, dwordAddress(T0),                                 \
            Immediate((Value1)&Mask##Size));                                   \
    __ mov(IceType_i32, dwordAddress(T0), Immediate(ValueIfFalse));            \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, dwordAddress(T0), Immediate(ValueIfTrue));             \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, uint32_t(Value0));                                     \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(((Value0)&Mask##Size) & ((Value1)&Mask##Size) ? ValueIfTrue      \
                                                            : ValueIfFalse,    \
              test.contentsOfDword(T0))                                        \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplValues(Dst, Value0, Src, Value1, Size)                         \
  do {                                                                         \
    TestImplRegReg(Dst, Value0, Src, Value1, Size);                            \
    TestImplRegImm(Dst, Value0, Value1, Size);                                 \
    TestImplAddrReg(Value0, Src, Value1, Size);                                \
    TestImplAddrImm(Value0, Value1, Size);                                     \
  } while (0)

#define TestImplSize(Dst, Src, Size)                                           \
  do {                                                                         \
    TestImplValues(Dst, 0xF0F12101, Src, 0x00000000, Size);                    \
    TestImplValues(Dst, 0xF0000000, Src, 0xF0000000, Size);                    \
    TestImplValues(Dst, 0x0F00000F, Src, 0xF00000F0, Size);                    \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSize(Dst, Src, 8);                                                 \
    TestImplSize(Dst, Src, 16);                                                \
    TestImplSize(Dst, Src, 32);                                                \
  } while (0)

  TestImpl(eax, ebx);
  TestImpl(ebx, ecx);
  TestImpl(ecx, edx);
  TestImpl(edx, esi);
  TestImpl(esi, edi);
  TestImpl(edi, eax);

#undef TestImpl
#undef TestImplSize
#undef TestImplValues
#undef TestImplAddrImm
#undef TestImplAddrReg
#undef TestImplRegImm
#undef TestImplRegReg
}

// No mull/div because x86.
// No shift because x86.
TEST_F(AssemblerX8632Test, Arith_most) {
  static constexpr uint32_t Mask8 = 0xFF;
  static constexpr uint32_t Mask16 = 0xFFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplRegReg(Inst, Dst, Value0, Src, Value1, Type, Size, Op)         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", " #Src ", " #Value1                \
        ", " #Type #Size "_t, " #Op ")";                                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(Value0));                                                 \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Value1));                                                 \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Mask##Size &static_cast<uint32_t>(                               \
                  static_cast<Type##Size##_t>((Value0)&Mask##Size)             \
                      Op static_cast<Type##Size##_t>((Value1)&Mask##Size)),    \
              Mask##Size &test.Dst())                                          \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Inst, Dst, Value0, Value1, Type, Size, Op)             \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", Addr, " #Value1 ", " #Type #Size   \
        "_t, " #Op ")";                                                        \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value1;                                                \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(Value0));                                                 \
    __ mov(IceType_i##Size, dwordAddress(T0), Immediate(Value1));              \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T0));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Mask##Size &static_cast<uint32_t>(                               \
                  static_cast<Type##Size##_t>((Value0)&Mask##Size)             \
                      Op static_cast<Type##Size##_t>((Value1)&Mask##Size)),    \
              Mask##Size &test.Dst())                                          \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplRegImm(Inst, Dst, Value0, Imm, Type, Size, Op)                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", Imm(" #Imm "), " #Type #Size       \
        "_t, " #Op ")";                                                        \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(Value0));                                                 \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            Immediate((Imm)&Mask##Size));                                      \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Mask##Size &static_cast<uint32_t>(                               \
                  static_cast<Type##Size##_t>((Value0)&Mask##Size)             \
                      Op static_cast<Type##Size##_t>((Imm)&Mask##Size)),       \
              Mask##Size &test.Dst())                                          \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplAddrReg(Inst, Value0, Src, Value1, Type, Size, Op)             \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", Addr, " #Value0 ", " #Src ", " #Value1 ", " #Type #Size   \
        "_t, " #Op ")";                                                        \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value0;                                                \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Value1));                                                 \
    __ Inst(IceType_i##Size, dwordAddress(T0),                                 \
            GPRRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Mask##Size &static_cast<uint32_t>(                               \
                  static_cast<Type##Size##_t>((Value0)&Mask##Size)             \
                      Op static_cast<Type##Size##_t>((Value1)&Mask##Size)),    \
              Mask##Size &test.contentsOfDword(T0))                            \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplAddrImm(Inst, Value0, Imm, Type, Size, Op)                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", Addr, " #Value0 ", Imm, " #Imm ", " #Type #Size           \
        "_t, " #Op ")";                                                        \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value0;                                                \
                                                                               \
    __ Inst(IceType_i##Size, dwordAddress(T0), Immediate((Imm)&Mask##Size));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Mask##Size &static_cast<uint32_t>(                               \
                  static_cast<Type##Size##_t>((Value0)&Mask##Size)             \
                      Op static_cast<Type##Size##_t>((Imm)&Mask##Size)),       \
              Mask##Size &test.contentsOfDword(T0))                            \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplOp(Inst, Dst, Value0, Src, Value1, Type, Size, Op)             \
  do {                                                                         \
    TestImplRegReg(Inst, Dst, Value0, Src, Value1, Type, Size, Op);            \
    TestImplRegAddr(Inst, Dst, Value0, Value1, Type, Size, Op);                \
    TestImplRegImm(Inst, Dst, Value0, Value1, Type, Size, Op);                 \
    TestImplAddrReg(Inst, Value0, Src, Value1, Type, Size, Op);                \
    TestImplAddrImm(Inst, Value0, Value1, Type, Size, Op);                     \
  } while (0)

#define TestImplValues(Dst, Value0, Src, Value1, Size)                         \
  do {                                                                         \
    TestImplOp(And, Dst, Value0, Src, Value1, int, Size, &);                   \
    TestImplOp(And, Dst, Value0, Src, Value1, uint, Size, &);                  \
    TestImplOp(Or, Dst, Value0, Src, Value1, int, Size, |);                    \
    TestImplOp(Or, Dst, Value0, Src, Value1, uint, Size, |);                   \
    TestImplOp(Xor, Dst, Value0, Src, Value1, int, Size, ^);                   \
    TestImplOp(Xor, Dst, Value0, Src, Value1, uint, Size, ^);                  \
    TestImplOp(add, Dst, Value0, Src, Value1, int, Size, +);                   \
    TestImplOp(add, Dst, Value0, Src, Value1, uint, Size, +);                  \
    TestImplOp(sub, Dst, Value0, Src, Value1, int, Size, -);                   \
    TestImplOp(sub, Dst, Value0, Src, Value1, uint, Size, -);                  \
  } while (0)

#define TestImplSize(Dst, Src, Size)                                           \
  do {                                                                         \
    TestImplValues(Dst, 0xF0F12101, Src, 0x00000000, Size);                    \
    TestImplValues(Dst, 0xF0000000, Src, 0xF0000000, Size);                    \
    TestImplValues(Dst, 0x0F00000F, Src, 0xF0000070, Size);                    \
    TestImplValues(Dst, 0x0F00F00F, Src, 0xF000F070, Size);                    \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    if (GPRRegister::Encoded_Reg_##Src <= 3 &&                                 \
        GPRRegister::Encoded_Reg_##Dst <= 3) {                                 \
      TestImplSize(Dst, Src, 8);                                               \
    }                                                                          \
    TestImplSize(Dst, Src, 16);                                                \
    TestImplSize(Dst, Src, 32);                                                \
  } while (0)

  TestImpl(eax, ebx);
  TestImpl(ebx, ecx);
  TestImpl(ecx, edx);
  TestImpl(edx, esi);
  TestImpl(esi, edi);
  TestImpl(edi, eax);

#undef TestImpl
#undef TestImplSize
#undef TestImplValues
#undef TestImplOp
#undef TestImplAddrImm
#undef TestImplAddrReg
#undef TestImplRegImm
#undef TestImplRegAddr
#undef TestImplRegReg
}

TEST_F(AssemblerX8632Test, Arith_BorrowNCarry) {
  const uint32_t Mask8 = 0x000000FF;
  const uint32_t Mask16 = 0x0000FFFF;
  const uint32_t Mask32 = 0xFFFFFFFF;

  const uint64_t ResultMask8 = 0x000000000000FFFFull;
  const uint64_t ResultMask16 = 0x00000000FFFFFFFFull;
  const uint64_t ResultMask32 = 0xFFFFFFFFFFFFFFFFull;

#define TestImplRegReg(Inst0, Inst1, Dst0, Dst1, Value0, Src0, Src1, Value1,   \
                       Op, Size)                                               \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32,                       \
                  "Invalid size " #Size);                                      \
    static constexpr char TestString[] =                                       \
        "(" #Inst0 ", " #Inst1 ", " #Dst0 ", " #Dst1 ", " #Value0 ", " #Src0   \
        ", " #Src1 ", " #Value1 ", " #Op ", " #Size ")";                       \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                   \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                   \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src0,                   \
           Immediate(uint64_t(Value1) & Mask##Size));                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src1,                   \
           Immediate((uint64_t(Value1) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                 \
             GPRRegister::Encoded_Reg_##Src0);                                 \
    __ Inst1(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                 \
             GPRRegister::Encoded_Reg_##Src1);                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    static constexpr uint64_t Result = (uint64_t(Value0) & ResultMask##Size)   \
        Op(uint64_t(Value1) & ResultMask##Size);                               \
    static constexpr uint32_t Expected0 = Result & Mask##Size;                 \
    static constexpr uint32_t Expected1 = (Result >> Size) & Mask##Size;       \
    ASSERT_EQ(Expected0, test.Dst0()) << TestString << ": 0";                  \
    ASSERT_EQ(Expected1, test.Dst1()) << TestString << ": 1";                  \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Inst0, Inst1, Dst0, Dst1, Value0, Value1, Op, Size)    \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32,                       \
                  "Invalid size " #Size);                                      \
    static constexpr char TestString[] =                                       \
        "(" #Inst0 ", " #Inst1 ", " #Dst0 ", " #Dst1 ", " #Value0              \
        ", Addr, " #Value1 ", " #Op ", " #Size ")";                            \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = uint64_t(Value1) & Mask##Size;                         \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = (uint64_t(Value1) >> Size) & Mask##Size;               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                   \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                   \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                 \
             dwordAddress(T0));                                                \
    __ Inst1(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                 \
             dwordAddress(T1));                                                \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.setDwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    static constexpr uint64_t Result = (uint64_t(Value0) & ResultMask##Size)   \
        Op(uint64_t(Value1) & ResultMask##Size);                               \
    static constexpr uint32_t Expected0 = Result & Mask##Size;                 \
    static constexpr uint32_t Expected1 = (Result >> Size) & Mask##Size;       \
    ASSERT_EQ(Expected0, test.Dst0()) << TestString << ": 0";                  \
    ASSERT_EQ(Expected1, test.Dst1()) << TestString << ": 1";                  \
    reset();                                                                   \
  } while (0)

#define TestImplRegImm(Inst0, Inst1, Dst0, Dst1, Value0, Imm, Op, Size)        \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32,                       \
                  "Invalid size " #Size);                                      \
    static constexpr char TestString[] =                                       \
        "(" #Inst0 ", " #Inst1 ", " #Dst0 ", " #Dst1 ", " #Value0              \
        ", Imm(" #Imm "), " #Op ", " #Size ")";                                \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                   \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                   \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst0,                 \
             Immediate(uint64_t(Imm) & Mask##Size));                           \
    __ Inst1(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst1,                 \
             Immediate((uint64_t(Imm) >> Size) & Mask##Size));                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    static constexpr uint64_t Result = (uint64_t(Value0) & ResultMask##Size)   \
        Op(uint64_t(Imm) & ResultMask##Size);                                  \
    static constexpr uint32_t Expected0 = Result & Mask##Size;                 \
    static constexpr uint32_t Expected1 = (Result >> Size) & Mask##Size;       \
    ASSERT_EQ(Expected0, test.Dst0()) << TestString << ": 0";                  \
    ASSERT_EQ(Expected1, test.Dst1()) << TestString << ": 1";                  \
    reset();                                                                   \
  } while (0)

#define TestImplAddrReg(Inst0, Inst1, Value0, Src0, Src1, Value1, Op, Size)    \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32,                       \
                  "Invalid size " #Size);                                      \
    static constexpr char TestString[] =                                       \
        "(" #Inst0 ", " #Inst1 ", Addr, " #Value0 ", " #Src0 ", " #Src1        \
        ", " #Value1 ", " #Op ", " #Size ")";                                  \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = uint64_t(Value0) & Mask##Size;                         \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = (uint64_t(Value0) >> Size) & Mask##Size;               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src0,                   \
           Immediate(uint64_t(Value1) & Mask##Size));                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src1,                   \
           Immediate((uint64_t(Value1) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, dwordAddress(T0),                                \
             GPRRegister::Encoded_Reg_##Src0);                                 \
    __ Inst1(IceType_i##Size, dwordAddress(T1),                                \
             GPRRegister::Encoded_Reg_##Src1);                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.setDwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    static constexpr uint64_t Result = (uint64_t(Value0) & ResultMask##Size)   \
        Op(uint64_t(Value1) & ResultMask##Size);                               \
    static constexpr uint32_t Expected0 = Result & Mask##Size;                 \
    static constexpr uint32_t Expected1 = (Result >> Size) & Mask##Size;       \
    ASSERT_EQ(Expected0, test.contentsOfDword(T0)) << TestString << ": 0";     \
    ASSERT_EQ(Expected1, test.contentsOfDword(T1)) << TestString << ": 1";     \
    reset();                                                                   \
  } while (0)

#define TestImplAddrImm(Inst0, Inst1, Value0, Imm, Op, Size)                   \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32,                       \
                  "Invalid size " #Size);                                      \
    static constexpr char TestString[] =                                       \
        "(" #Inst0 ", " #Inst1 ", Addr, " #Value0 ", Imm(" #Imm "), " #Op      \
        ", " #Size ")";                                                        \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = uint64_t(Value0) & Mask##Size;                         \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = (uint64_t(Value0) >> Size) & Mask##Size;               \
    __ Inst0(IceType_i##Size, dwordAddress(T0),                                \
             Immediate(uint64_t(Imm) & Mask##Size));                           \
    __ Inst1(IceType_i##Size, dwordAddress(T1),                                \
             Immediate((uint64_t(Imm) >> Size) & Mask##Size));                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.setDwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    static constexpr uint64_t Result = (uint64_t(Value0) & ResultMask##Size)   \
        Op(uint64_t(Imm) & ResultMask##Size);                                  \
    static constexpr uint32_t Expected0 = Result & Mask##Size;                 \
    static constexpr uint32_t Expected1 = (Result >> Size) & Mask##Size;       \
    ASSERT_EQ(Expected0, test.contentsOfDword(T0)) << TestString << ": 0";     \
    ASSERT_EQ(Expected1, test.contentsOfDword(T1)) << TestString << ": 1";     \
    reset();                                                                   \
  } while (0)

#define TestImplOp(Inst0, Inst1, Dst0, Dst1, Value0, Src0, Src1, Value1, Op,   \
                   Size)                                                       \
  do {                                                                         \
    TestImplRegReg(Inst0, Inst1, Dst0, Dst1, Value0, Src0, Src1, Value1, Op,   \
                   Size);                                                      \
    TestImplRegAddr(Inst0, Inst1, Dst0, Dst1, Value0, Value1, Op, Size);       \
    TestImplRegImm(Inst0, Inst1, Dst0, Dst1, Value0, Value1, Op, Size);        \
    TestImplAddrReg(Inst0, Inst1, Value0, Src0, Src1, Value1, Op, Size);       \
    TestImplAddrImm(Inst0, Inst1, Value0, Value1, Op, Size);                   \
  } while (0)

#define TestImplValues(Dst0, Dst1, Value0, Src0, Src1, Value1, Size)           \
  do {                                                                         \
    TestImplOp(add, adc, Dst0, Dst1, Value0, Src0, Src1, Value1, +, Size);     \
    TestImplOp(sub, sbb, Dst0, Dst1, Value0, Src0, Src1, Value1, -, Size);     \
  } while (0)

#define TestImplSize(Dst0, Dst1, Src0, Src1, Size)                             \
  do {                                                                         \
    TestImplValues(Dst0, Dst1, 0xFFFFFFFFFFFFFF00ull, Src0, Src1,              \
                   0xFFFFFFFF0000017Full, Size);                               \
  } while (0)

#define TestImpl(Dst0, Dst1, Src0, Src1)                                       \
  do {                                                                         \
    if (GPRRegister::Encoded_Reg_##Dst0 <= 3 &&                                \
        GPRRegister::Encoded_Reg_##Dst1 <= 3 &&                                \
        GPRRegister::Encoded_Reg_##Src0 <= 3 &&                                \
        GPRRegister::Encoded_Reg_##Src1 <= 3) {                                \
      TestImplSize(Dst0, Dst1, Src0, Src1, 8);                                 \
    }                                                                          \
    TestImplSize(Dst0, Dst1, Src0, Src1, 16);                                  \
    TestImplSize(Dst0, Dst1, Src0, Src1, 32);                                  \
  } while (0)

  TestImpl(eax, ebx, ecx, edx);
  TestImpl(ebx, ecx, edx, esi);
  TestImpl(ecx, edx, esi, edi);
  TestImpl(edx, esi, edi, eax);
  TestImpl(esi, edi, eax, ebx);
  TestImpl(edi, eax, ebx, ecx);

#undef TestImpl
#undef TestImplSize
#undef TestImplValues
#undef TestImplOp
#undef TestImplAddrImm
#undef TestImplAddrReg
#undef TestImplRegImm
#undef TestImplRegAddr
#undef TestImplRegReg
}

TEST_F(AssemblerX8632LowLevelTest, Cbw_Cwd_Cdq) {
#define TestImpl(Inst, BytesSize, ...)                                         \
  do {                                                                         \
    __ Inst();                                                                 \
    ASSERT_EQ(BytesSize, codeBytesSize()) << #Inst;                            \
    verifyBytes<BytesSize>(codeBytes(), __VA_ARGS__);                          \
    reset();                                                                   \
  } while (0)

  TestImpl(cbw, 2u, 0x66, 0x98);
  TestImpl(cwd, 2u, 0x66, 0x99);
  TestImpl(cdq, 1u, 0x99);

#undef TestImpl
}

TEST_F(AssemblerX8632Test, SingleOperandMul) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplReg(Inst, Value0, Src, Value1, Type, Size)                     \
  do {                                                                         \
    static_assert(GPRRegister::Encoded_Reg_eax !=                              \
                      GPRRegister::Encoded_Reg_##Src,                          \
                  "eax can not be src1.");                                     \
                                                                               \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Value0 ", " #Src ", " #Value1 ", " #Type ", " #Size    \
        ")";                                                                   \
    static constexpr Type##64_t OperandEax =                                   \
        static_cast<Type##Size##_t>((Value0)&Mask##Size);                      \
    static constexpr Type##64_t OperandOther =                                 \
        static_cast<Type##Size##_t>((Value1)&Mask##Size);                      \
    static constexpr uint32_t ExpectedEax =                                    \
        Mask##Size & (OperandEax * OperandOther);                              \
    static constexpr uint32_t ExpectedEdx =                                    \
        Mask##Size & ((OperandEax * OperandOther) >> Size);                    \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_eax,                      \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate((Value1)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Src);                  \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
      if (GPRRegister::Encoded_Reg_##Src == GPRRegister::Encoded_Reg_esi) {    \
        /* src == dh; clear dx's upper 8 bits. */                              \
        __ And(IceType_i16, GPRRegister::Encoded_Reg_edx, Immediate(0x00FF));  \
      }                                                                        \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(ExpectedEax, test.eax()) << TestString;                          \
    ASSERT_EQ(ExpectedEdx, test.edx()) << TestString;                          \
    reset();                                                                   \
  } while (0)

#define TestImplAddr(Inst, Value0, Value1, Type, Size)                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Value0 ", Addr, " #Value1 ", " #Type ", " #Size ")";   \
    static const uint32_t T0 = allocateDword();                                \
    static constexpr uint32_t V0 = Value1;                                     \
    static constexpr Type##64_t OperandEax =                                   \
        static_cast<Type##Size##_t>((Value0)&Mask##Size);                      \
    static constexpr Type##64_t OperandOther =                                 \
        static_cast<Type##Size##_t>((Value1)&Mask##Size);                      \
    static constexpr uint32_t ExpectedEax =                                    \
        Mask##Size & (OperandEax * OperandOther);                              \
    static constexpr uint32_t ExpectedEdx =                                    \
        Mask##Size & ((OperandEax * OperandOther) >> Size);                    \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_eax,                      \
           Immediate((Value0)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, dwordAddress(T0));                                \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(ExpectedEax, test.eax()) << TestString;                          \
    ASSERT_EQ(ExpectedEdx, test.edx()) << TestString;                          \
    reset();                                                                   \
  } while (0)

#define TestImplOp(Inst, Value0, Src, Value1, Type, Size)                      \
  do {                                                                         \
    TestImplReg(Inst, Value0, Src, Value1, Type, Size);                        \
    TestImplAddr(Inst, Value0, Value1, Type, Size);                            \
  } while (0)

#define TestImplValue(Value0, Src, Value1, Size)                               \
  do {                                                                         \
    TestImplOp(mul, Value0, Src, Value1, uint, Size);                          \
    TestImplOp(imul, Value0, Src, Value1, int, Size);                          \
  } while (0)

#define TestImplSize(Src, Size)                                                \
  do {                                                                         \
    TestImplValue(10, Src, 1, Size);                                           \
    TestImplValue(10, Src, -1, Size);                                          \
    TestImplValue(-10, Src, 37, Size);                                         \
    TestImplValue(-10, Src, -15, Size);                                        \
  } while (0)

#define TestImpl(Src)                                                          \
  do {                                                                         \
    TestImplSize(Src, 8);                                                      \
    TestImplSize(Src, 16);                                                     \
    TestImplSize(Src, 32);                                                     \
  } while (0)

  TestImpl(ebx);
  TestImpl(ecx);
  TestImpl(edx);
  TestImpl(esi);
  TestImpl(edi);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplOp
#undef TestImplAddr
#undef TestImplReg
}

TEST_F(AssemblerX8632Test, TwoOperandImul) {
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplRegReg(Dst, Value0, Src, Value1, Size)                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Size ")";           \
    static constexpr int64_t Operand0 =                                        \
        static_cast<int##Size##_t>((Value0)&Mask##Size);                       \
    static constexpr int64_t Operand1 =                                        \
        static_cast<int##Size##_t>((Value1)&Mask##Size);                       \
    static constexpr uint32_t Expected = Mask##Size & (Operand0 * Operand1);   \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate((Value1)&Mask##Size));                                    \
    __ imul(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
      if (GPRRegister::Encoded_Reg_##Src == GPRRegister::Encoded_Reg_esi) {    \
        /* src == dh; clear dx's upper 8 bits. */                              \
        __ And(IceType_i16, GPRRegister::Encoded_Reg_edx, Immediate(0x00FF));  \
      }                                                                        \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestImplRegImm(Dst, Value0, Imm, Size)                                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Imm(" #Imm "), " #Size ")";                   \
    static constexpr int64_t Operand0 =                                        \
        static_cast<int##Size##_t>((Value0)&Mask##Size);                       \
    static constexpr int64_t Operand1 =                                        \
        static_cast<int##Size##_t>((Imm)&Mask##Size);                          \
    static constexpr uint32_t Expected = Mask##Size & (Operand0 * Operand1);   \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ imul(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst, Immediate(Imm));  \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Dst, Value0, Value1, Size)                             \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr," #Value1 ", " #Size ")";                \
    static constexpr int64_t Operand0 =                                        \
        static_cast<int##Size##_t>((Value0)&Mask##Size);                       \
    static constexpr int64_t Operand1 =                                        \
        static_cast<int##Size##_t>((Value1)&Mask##Size);                       \
    static constexpr uint32_t Expected = Mask##Size & (Operand0 * Operand1);   \
    const uint32_t T0 = allocateDword();                                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ imul(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T0));                                                 \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, static_cast<uint32_t>(Operand1));                      \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestImplValue(Dst, Value0, Src, Value1, Size)                          \
  do {                                                                         \
    TestImplRegReg(Dst, Value0, Src, Value1, Size);                            \
    TestImplRegImm(Dst, Value0, Value1, Size);                                 \
    TestImplRegAddr(Dst, Value0, Value1, Size);                                \
  } while (0)

#define TestImplSize(Dst, Src, Size)                                           \
  do {                                                                         \
    TestImplValue(Dst, 1, Src, 1, Size);                                       \
    TestImplValue(Dst, -10, Src, 0x4050AA20, Size);                            \
    TestImplValue(Dst, -2, Src, -55, Size);                                    \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSize(Dst, Src, 16);                                                \
    TestImplSize(Dst, Src, 32);                                                \
  } while (0)

  TestImpl(eax, ebx);
  TestImpl(ebx, ecx);
  TestImpl(ecx, edx);
  TestImpl(edx, esi);
  TestImpl(esi, edi);
  TestImpl(edi, eax);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplRegAddr
#undef TestImplRegImm
#undef TestImplRegReg
}

TEST_F(AssemblerX8632Test, Div) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

  static constexpr uint64_t Operand0Mask8 = 0x00000000000000FFull;
  static constexpr uint64_t Operand0Mask16 = 0x00000000FFFFFFFFull;
  static constexpr uint64_t Operand0Mask32 = 0xFFFFFFFFFFFFFFFFull;

  using Operand0Type_int8 = int16_t;
  using Operand0Type_uint8 = uint16_t;
  using Operand0Type_int16 = int32_t;
  using Operand0Type_uint16 = uint32_t;
  using Operand0Type_int32 = int64_t;
  using Operand0Type_uint32 = uint64_t;

#define TestImplReg(Inst, Value0, Src, Value1, Type, Size)                     \
  do {                                                                         \
    static_assert(GPRRegister::Encoded_Reg_eax !=                              \
                      GPRRegister::Encoded_Reg_##Src,                          \
                  "eax can not be src1.");                                     \
    static_assert(GPRRegister::Encoded_Reg_edx !=                              \
                      GPRRegister::Encoded_Reg_##Src,                          \
                  "edx can not be src1.");                                     \
                                                                               \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Value0 ", " #Src ", " #Value1 ", " #Type ", " #Size    \
        ")";                                                                   \
    static constexpr Operand0Type_##Type##Size Operand0 =                      \
        static_cast<Type##64_t>(Value0) & Operand0Mask##Size;                  \
    static constexpr Type##Size##_t Operand0Lo = Operand0 & Mask##Size;        \
    static constexpr Type##Size##_t Operand0Hi =                               \
        (Operand0 >> Size) & Mask##Size;                                       \
    static constexpr Type##Size##_t Operand1 =                                 \
        static_cast<Type##Size##_t>(Value1) & Mask##Size;                      \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_eax,                      \
           Immediate(Operand0Lo));                                             \
    if (Size == 8) {                                                           \
      /* mov Operand0Hi, %ah */                                                \
      __ mov(IceType_i8, GPRRegister::Encoded_Reg_esp, Immediate(Operand0Hi)); \
    } else {                                                                   \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             Immediate(Operand0Hi));                                           \
    }                                                                          \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Operand1));                                               \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Src);                  \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
      if (GPRRegister::Encoded_Reg_##Src == GPRRegister::Encoded_Reg_esi) {    \
        __ And(IceType_i16, GPRRegister::Encoded_Reg_edx, Immediate(0x00FF));  \
      }                                                                        \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    static constexpr uint32_t Quocient = (Operand0 / Operand1) & Mask##Size;   \
    static constexpr uint32_t Reminder = (Operand0 % Operand1) & Mask##Size;   \
    EXPECT_EQ(Quocient, test.eax()) << TestString;                             \
    EXPECT_EQ(Reminder, test.edx()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestImplAddr(Inst, Value0, Value1, Type, Size)                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Value0 ", Addr, " #Value1 ", " #Type ", " #Size ")";   \
    static constexpr Operand0Type_##Type##Size Operand0 =                      \
        static_cast<Type##64_t>(Value0) & Operand0Mask##Size;                  \
    static constexpr Type##Size##_t Operand0Lo = Operand0 & Mask##Size;        \
    static constexpr Type##Size##_t Operand0Hi =                               \
        (Operand0 >> Size) & Mask##Size;                                       \
    const uint32_t T0 = allocateDword();                                       \
    static constexpr Type##Size##_t V0 =                                       \
        static_cast<Type##Size##_t>(Value1) & Mask##Size;                      \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_eax,                      \
           Immediate(Operand0Lo));                                             \
    if (Size == 8) {                                                           \
      /* mov Operand0Hi, %ah */                                                \
      __ mov(IceType_i8, GPRRegister::Encoded_Reg_esp, Immediate(Operand0Hi)); \
    } else {                                                                   \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             Immediate(Operand0Hi));                                           \
    }                                                                          \
    __ Inst(IceType_i##Size, dwordAddress(T0));                                \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_edx,                    \
             GPRRegister::Encoded_Reg_esp);                                    \
      __ And(IceType_i16, GPRRegister::Encoded_Reg_eax, Immediate(0x00FF));    \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, static_cast<uint32_t>(V0));                            \
    test.run();                                                                \
                                                                               \
    static constexpr uint32_t Quocient = (Operand0 / V0) & Mask##Size;         \
    static constexpr uint32_t Reminder = (Operand0 % V0) & Mask##Size;         \
    EXPECT_EQ(Quocient, test.eax()) << TestString;                             \
    EXPECT_EQ(Reminder, test.edx()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestImplOp(Inst, Value0, Src, Value1, Type, Size)                      \
  do {                                                                         \
    TestImplReg(Inst, Value0, Src, Value1, Type, Size);                        \
    TestImplAddr(Inst, Value0, Value1, Type, Size);                            \
  } while (0)

#define TestImplValue(Value0, Src, Value1, Size)                               \
  do {                                                                         \
    TestImplOp(div, Value0, Src, Value1, uint, Size);                          \
    TestImplOp(idiv, Value0, Src, Value1, int, Size);                          \
  } while (0)

#define TestImplSize(Src, Size)                                                \
  do {                                                                         \
    TestImplValue(10, Src, 1, Size);                                           \
    TestImplValue(10, Src, -1, Size);                                          \
  } while (0)

#define TestImpl(Src)                                                          \
  do {                                                                         \
    TestImplSize(Src, 8);                                                      \
    TestImplSize(Src, 16);                                                     \
    TestImplSize(Src, 32);                                                     \
  } while (0)

  TestImpl(ebx);
  TestImpl(ecx);
  TestImpl(esi);
  TestImpl(edi);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplOp
#undef TestImplAddr
#undef TestImplReg
}

// This is not executable in x86-64 because the one byte inc/dec instructions
// became the REX prefixes. Therefore, these are tested with the low-level test
// infrastructure.
TEST_F(AssemblerX8632LowLevelTest, Incl_Decl_Reg) {
#define TestImpl(Inst, Dst, BaseOpcode)                                        \
  do {                                                                         \
    __ Inst(GPRRegister::Encoded_Reg_##Dst);                                   \
    static constexpr uint8_t ByteCount = 1;                                    \
    ASSERT_EQ(ByteCount, codeBytesSize());                                     \
    verifyBytes<ByteCount>(codeBytes(),                                        \
                           BaseOpcode | GPRRegister::Encoded_Reg_##Dst);       \
    reset();                                                                   \
  } while (0)

#define TestInc(Dst)                                                           \
  do {                                                                         \
    constexpr uint8_t InclOpcode = 0x40;                                       \
    TestImpl(incl, Dst, InclOpcode);                                           \
  } while (0)

#define TestDec(Dst)                                                           \
  do {                                                                         \
    constexpr uint8_t DeclOpcode = 0x48;                                       \
    TestImpl(decl, Dst, DeclOpcode);                                           \
  } while (0)

  TestInc(eax);
  TestInc(ecx);
  TestInc(edx);
  TestInc(ebx);
  TestInc(esp);
  TestInc(ebp);
  TestInc(esi);
  TestInc(esi);

  TestDec(eax);
  TestDec(ecx);
  TestDec(edx);
  TestDec(ebx);
  TestDec(esp);
  TestDec(ebp);
  TestDec(esi);
  TestDec(esi);

#undef TestInc
#undef TestDec
#undef TestImpl
}

TEST_F(AssemblerX8632Test, Incl_Decl_Addr) {
#define TestImpl(Inst, Value0)                                                 \
  do {                                                                         \
    const bool IsInc = std::string(#Inst).find("incl") != std::string::npos;   \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value0;                                                \
                                                                               \
    __ Inst(dwordAddress(T0));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Value0 + (IsInc ? 1 : -1)),                \
              test.contentsOfDword(T0));                                       \
    reset();                                                                   \
  } while (0)

#define TestInc(Value0)                                                        \
  do {                                                                         \
    TestImpl(incl, Value0);                                                    \
  } while (0)

#define TestDec(Value0)                                                        \
  do {                                                                         \
    TestImpl(decl, Value0);                                                    \
  } while (0)

  TestInc(230);

  TestDec(30);

#undef TestInc
#undef TestDec
#undef TestImpl
}

TEST_F(AssemblerX8632Test, Shifts) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplRegImm(Inst, Dst, Value0, Imm, Op, Type, Size)                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", Imm(" #Imm "), " #Op ", " #Type    \
        ", " #Size ")";                                                        \
    const bool IsRol = std::string(#Inst).find("rol") != std::string::npos;    \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op(Imm) |            \
                      (!IsRol ? 0 : (Value0) >> (Size - Imm)));                \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            Immediate((Imm)&Mask##Size));                                      \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected), test.Dst()) << TestString;      \
    reset();                                                                   \
  } while (0)

#define TestImplRegRegImm(Inst, Dst, Value0, Src, Value1, Count, Op0, Op1,     \
                          Type, Size)                                          \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", " #Src ", " #Value1                \
        ", Imm(" #Count "), " #Op0 ", " #Op1 ", " #Type ", " #Size ")";        \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op0(Count) |         \
                      (static_cast<Type##64_t>(Value1) Op1(Size - Count)));    \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate((Value1)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src, Immediate(Count));                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected), test.Dst()) << TestString;      \
    reset();                                                                   \
  } while (0)

#define TestImplRegCl(Inst, Dst, Value0, Count, Op, Type, Size)                \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", " #Count ", " #Op ", " #Type       \
        ", " #Size ")";                                                        \
    const bool IsRol = std::string(#Inst).find("rol") != std::string::npos;    \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op(Count) |          \
                      (!IsRol ? 0 : Value0 >> (Size - Count)));                \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i8, GPRRegister::Encoded_Reg_ecx,                           \
           Immediate((Count)&Mask##Size));                                     \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_ecx);                                     \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected), test.Dst()) << TestString;      \
    reset();                                                                   \
  } while (0)

#define TestImplRegRegCl(Inst, Dst, Value0, Src, Value1, Count, Op0, Op1,      \
                         Type, Size)                                           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Count    \
        ", " #Op0 ", " #Op1 ", " #Type ", " #Size ")";                         \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op0(Count) |         \
                      (static_cast<Type##64_t>(Value1) Op1(Size - Count)));    \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate((Value1)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_ecx,                      \
           Immediate((Count)&0x7F));                                           \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected), test.Dst()) << TestString;      \
    reset();                                                                   \
  } while (0)

#define TestImplAddrCl(Inst, Value0, Count, Op, Type, Size)                    \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", Addr, " #Value0 ", " #Count ", " #Op ", " #Type           \
        ", " #Size ")";                                                        \
    const bool IsRol = std::string(#Inst).find("rol") != std::string::npos;    \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op(Count) |          \
                      (!IsRol ? 0 : Value0 >> (Size - Count)));                \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value0;                                                \
                                                                               \
    __ mov(IceType_i8, GPRRegister::Encoded_Reg_ecx,                           \
           Immediate((Count)&Mask##Size));                                     \
    __ Inst(IceType_i##Size, dwordAddress(T0), GPRRegister::Encoded_Reg_ecx);  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected),                                 \
              Mask##Size &test.contentsOfDword(T0))                            \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplAddrRegCl(Inst, Value0, Src, Value1, Count, Op0, Op1, Type,    \
                          Size)                                                \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", Addr, " #Value0 ", " #Src ", " #Value1 ", " #Count        \
        ", " #Op0 ", " #Op1 ", " #Type ", " #Size ")";                         \
    const uint##Size##_t Expected =                                            \
        Mask##Size & (static_cast<Type##Size##_t>(Value0) Op0(Count) |         \
                      (static_cast<Type##64_t>(Value1) Op1(Size - Count)));    \
    const uint32_t T0 = allocateDword();                                       \
                                                                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate((Value1)&Mask##Size));                                    \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_ecx,                      \
           Immediate((Count)&0x7F));                                           \
    __ Inst(IceType_i##Size, dwordAddress(T0),                                 \
            GPRRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, static_cast<uint32_t>(Value0));                        \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Expected), test.contentsOfDword(T0))       \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplOp(Inst, Dst, Value0, Count, Op, Type, Size)                   \
  do {                                                                         \
    static_assert(GPRRegister::Encoded_Reg_##Dst !=                            \
                      GPRRegister::Encoded_Reg_ecx,                            \
                  "ecx should not be specified as Dst");                       \
    TestImplRegImm(Inst, Dst, Value0, Count, Op, Type, Size);                  \
    TestImplRegImm(Inst, ecx, Value0, Count, Op, Type, Size);                  \
    TestImplRegCl(Inst, Dst, Value0, Count, Op, Type, Size);                   \
    TestImplAddrCl(Inst, Value0, Count, Op, Type, Size);                       \
  } while (0)

#define TestImplThreeOperandOp(Inst, Dst, Value0, Src, Value1, Count, Op0,     \
                               Op1, Type, Size)                                \
  do {                                                                         \
    static_assert(GPRRegister::Encoded_Reg_##Dst !=                            \
                      GPRRegister::Encoded_Reg_ecx,                            \
                  "ecx should not be specified as Dst");                       \
    static_assert(GPRRegister::Encoded_Reg_##Src !=                            \
                      GPRRegister::Encoded_Reg_ecx,                            \
                  "ecx should not be specified as Src");                       \
    TestImplRegRegImm(Inst, Dst, Value0, Src, Value1, Count, Op0, Op1, Type,   \
                      Size);                                                   \
    TestImplRegRegCl(Inst, Dst, Value0, Src, Value1, Count, Op0, Op1, Type,    \
                     Size);                                                    \
    TestImplAddrRegCl(Inst, Value0, Src, Value1, Count, Op0, Op1, Type, Size); \
  } while (0)

#define TestImplValue(Dst, Value0, Count, Size)                                \
  do {                                                                         \
    TestImplOp(rol, Dst, Value0, Count, <<, uint, Size);                       \
    TestImplOp(shl, Dst, Value0, Count, <<, uint, Size);                       \
    TestImplOp(shr, Dst, Value0, Count, >>, uint, Size);                       \
    TestImplOp(sar, Dst, Value0, Count, >>, int, Size);                        \
  } while (0)

#define TestImplThreeOperandValue(Dst, Value0, Src, Value1, Count, Size)       \
  do {                                                                         \
    TestImplThreeOperandOp(shld, Dst, Value0, Src, Value1, Count, <<, >>,      \
                           uint, Size);                                        \
    TestImplThreeOperandOp(shrd, Dst, Value0, Src, Value1, Count, >>, <<,      \
                           uint, Size);                                        \
  } while (0)

#define TestImplSize(Dst, Size)                                                \
  do {                                                                         \
    TestImplValue(Dst, 0x8F, 3, Size);                                         \
    TestImplValue(Dst, 0x8FFF, 7, Size);                                       \
    TestImplValue(Dst, 0x8FFFF, 7, Size);                                      \
  } while (0)

#define TestImplThreeOperandSize(Dst, Src, Size)                               \
  do {                                                                         \
    TestImplThreeOperandValue(Dst, 0xFFF3, Src, 0xA000, 8, Size);              \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    if (GPRRegister::Encoded_Reg_##Dst < 4) {                                  \
      TestImplSize(Dst, 8);                                                    \
    }                                                                          \
    TestImplSize(Dst, 16);                                                     \
    TestImplThreeOperandSize(Dst, Src, 16);                                    \
    TestImplSize(Dst, 32);                                                     \
    TestImplThreeOperandSize(Dst, Src, 32);                                    \
  } while (0)

  TestImpl(eax, ebx);
  TestImpl(ebx, edx);
  TestImpl(edx, esi);
  TestImpl(esi, edi);
  TestImpl(edi, eax);

#undef TestImpl
#undef TestImplThreeOperandSize
#undef TestImplSize
#undef TestImplValue
#undef TestImplThreeOperandValue
#undef TestImplOp
#undef TestImplThreeOperandOp
#undef TestImplAddrCl
#undef TestImplRegRegCl
#undef TestImplRegCl
#undef TestImplRegRegImm
#undef TestImplRegImm
}

TEST_F(AssemblerX8632Test, Neg) {
  static constexpr uint32_t Mask8 = 0x000000ff;
  static constexpr uint32_t Mask16 = 0x0000ffff;
  static constexpr uint32_t Mask32 = 0xffffffff;

#define TestImplReg(Dst, Size)                                                 \
  do {                                                                         \
    static constexpr int32_t Value = 0xFF00A543;                               \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                    \
           Immediate(static_cast<int##Size##_t>(Value) & Mask##Size));         \
    __ neg(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst);                   \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_eax,                      \
           GPRRegister::Encoded_Reg_##Dst);                                    \
    __ And(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(Mask##Size));  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(1 + (~static_cast<int##Size##_t>(Value) & Mask##Size),           \
              test.eax())                                                      \
        << "(" #Dst ", " #Size ")";                                            \
    reset();                                                                   \
  } while (0)

#define TestImplAddr(Size)                                                     \
  do {                                                                         \
    static constexpr int32_t Value = 0xFF00A543;                               \
    const uint32_t T0 = allocateDword();                                       \
    __ neg(IceType_i##Size, dwordAddress(T0));                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, Value &Mask##Size);                                    \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(1 + (~static_cast<int##Size##_t>(Value) & Mask##Size),           \
              test.contentsOfDword(T0))                                        \
        << "(Addr, " #Size ")";                                                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Size)                                                         \
  do {                                                                         \
    TestImplAddr(Size);                                                        \
    TestImplReg(eax, Size);                                                    \
    TestImplReg(ebx, Size);                                                    \
    TestImplReg(ecx, Size);                                                    \
    TestImplReg(edx, Size);                                                    \
    TestImplReg(esi, Size);                                                    \
    TestImplReg(edi, Size);                                                    \
  } while (0)

  TestImpl(8);
  TestImpl(16);
  TestImpl(32);

#undef TestImpl
#undef TestImplAddr
#undef TestImplReg
}

TEST_F(AssemblerX8632Test, Not) {
#define TestImpl(Dst)                                                          \
  do {                                                                         \
    static constexpr uint32_t Value = 0xFF00A543;                              \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst, Immediate(Value));     \
    __ notl(GPRRegister::Encoded_Reg_##Dst);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(~Value, test.Dst()) << "(" #Dst ")";                             \
    reset();                                                                   \
  } while (0)

  TestImpl(eax);
  TestImpl(ebx);
  TestImpl(ecx);
  TestImpl(edx);
  TestImpl(esi);
  TestImpl(edi);

#undef TestImpl
}

TEST_F(AssemblerX8632Test, Bswap) {
#define TestImpl(Dst)                                                          \
  do {                                                                         \
    static constexpr uint32_t Value = 0xFF00A543;                              \
    static constexpr uint32_t Expected = 0x43A500FF;                           \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst, Immediate(Value));     \
    __ bswap(IceType_i32, GPRRegister::Encoded_Reg_##Dst);                     \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst()) << "(" #Dst ")";                           \
    reset();                                                                   \
  } while (0)

  TestImpl(eax);
  TestImpl(ebx);
  TestImpl(ecx);
  TestImpl(edx);
  TestImpl(esi);
  TestImpl(edi);

#undef TestImpl
}

TEST_F(AssemblerX8632Test, Bt) {
#define TestImpl(Dst, Value0, Src, Value1)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ")";                      \
    static constexpr uint32_t Expected = ((Value0) & (1u << (Value1))) != 0;   \
                                                                               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dst, Immediate(Value0));    \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src, Immediate(Value1));    \
    __ bt(GPRRegister::Encoded_Reg_##Dst, GPRRegister::Encoded_Reg_##Src);     \
    __ setcc(Cond::Br_b, ByteRegister::Encoded_8_Reg_al);                      \
    __ And(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0xFFu));       \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.eax()) << TestString;                             \
    reset();                                                                   \
  } while (0)

  TestImpl(eax, 0x08000000, ebx, 27u);
  TestImpl(ebx, 0x08000000, ecx, 23u);
  TestImpl(ecx, 0x00000000, edx, 1u);
  TestImpl(edx, 0x08000300, esi, 9u);
  TestImpl(esi, 0x08000300, edi, 10u);
  TestImpl(edi, 0x7FFFEFFF, eax, 13u);

#undef TestImpl
}

template <uint32_t Value, uint32_t Bits> class BitScanHelper {
  BitScanHelper() = delete;

public:
  static_assert(Bits == 16 || Bits == 32, "Bits must be 16 or 32");
  using ValueType =
      typename std::conditional<Bits == 16, uint16_t, uint32_t>::type;

private:
  static constexpr ValueType BitIndex(bool Forward, ValueType Index) {
    return (Value == 0)
               ? BitScanHelper<Value, Bits>::NoBitSet
               : (Value & (1u << Index)
                      ? Index
                      : BitIndex(Forward, (Forward ? Index + 1 : Index - 1)));
  }

public:
  static constexpr ValueType NoBitSet = static_cast<ValueType>(-1);
  static constexpr ValueType bsf = BitIndex(/*Forward*/ true, /*Index=*/0);
  static constexpr ValueType bsr =
      BitIndex(/*Forward*/ false, /*Index=*/Bits - 1);
};

TEST_F(AssemblerX8632Test, BitScanOperations) {
#define TestImplRegReg(Inst, Dst, Src, Value1, Size)                           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Src ", " #Value1 ", " #Size ")";             \
    static constexpr uint32_t Expected = BitScanHelper<Value1, Size>::Inst;    \
    const uint32_t ZeroFlag = allocateDword();                                 \
    __ mov(IceType_i##Size, GPRRegister::Encoded_Reg_##Src,                    \
           Immediate(Value1));                                                 \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            GPRRegister::Encoded_Reg_##Src);                                   \
    __ setcc(Cond::Br_e, dwordAddress(ZeroFlag));                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(ZeroFlag, 0u);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((Expected == BitScanHelper<Value1, Size>::NoBitSet),             \
              test.contentsOfDword(ZeroFlag))                                  \
        << TestString;                                                         \
    if ((Expected != BitScanHelper<Value1, Size>::NoBitSet)) {                 \
      ASSERT_EQ(Expected, test.Dst()) << TestString;                           \
    }                                                                          \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Inst, Dst, Value1, Size)                               \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", Addr, " #Value1 ", " #Size ")";                 \
    static constexpr uint32_t Expected = BitScanHelper<Value1, Size>::Inst;    \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t ZeroFlag = allocateDword();                                 \
    __ Inst(IceType_i##Size, GPRRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T0));                                                 \
    __ setcc(Cond::Br_e, dwordAddress(ZeroFlag));                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, Value1);                                               \
    test.setDwordTo(ZeroFlag, 0u);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((Expected == BitScanHelper<Value1, Size>::NoBitSet),             \
              test.contentsOfDword(ZeroFlag))                                  \
        << TestString;                                                         \
    if (Expected != BitScanHelper<Value1, Size>::NoBitSet) {                   \
      ASSERT_EQ(Expected, test.Dst()) << TestString;                           \
    }                                                                          \
    reset();                                                                   \
  } while (0)

#define TestImplSize(Dst, Src, Value1, Size)                                   \
  do {                                                                         \
    TestImplRegReg(bsf, Dst, Src, Value1, Size);                               \
    TestImplRegAddr(bsf, Dst, Value1, Size);                                   \
    TestImplRegReg(bsr, Dst, Src, Value1, Size);                               \
    TestImplRegAddr(bsf, Dst, Value1, Size);                                   \
  } while (0)

#define TestImplValue(Dst, Src, Value1)                                        \
  do {                                                                         \
    TestImplSize(Dst, Src, Value1, 16);                                        \
    TestImplSize(Dst, Src, Value1, 32);                                        \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplValue(Dst, Src, 0x80000001);                                       \
    TestImplValue(Dst, Src, 0x00000000);                                       \
    TestImplValue(Dst, Src, 0x80001000);                                       \
    TestImplValue(Dst, Src, 0x00FFFF00);                                       \
  } while (0)

  TestImpl(eax, ebx);
  TestImpl(ebx, ecx);
  TestImpl(ecx, edx);
  TestImpl(edx, esi);
  TestImpl(esi, edi);
  TestImpl(edi, eax);

#undef TestImpl
#undef TestImplValue
#undef TestImplSize
#undef TestImplRegAddr
#undef TestImplRegReg
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice
