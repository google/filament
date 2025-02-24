//===- subzero/unittest/AssemblerX8664/GPRArith.cpp -----------------------===//
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

TEST_F(AssemblerX8664Test, PopAddr) {
  const uint32_t T0 = allocateQword();
  constexpr uint64_t V0 = 0x3AABBEFABBBAA3ull;

  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0xC0FFEE));
  __ pushl(GPRRegister::Encoded_Reg_eax);
  __ popl(dwordAddress(T0));

  AssembledTest test = assemble();
  test.setQwordTo(T0, V0);

  test.run();

  ASSERT_EQ(0xC0FFEEul, test.contentsOfQword(T0));
}

TEST_F(AssemblerX8664Test, SetCC) {
#define TestSetCC(C, Dest, IsTrue, Src0, Value0, Src1, Value1)                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #C ", " #Dest ", " #IsTrue ", " #Src0 ", " #Value0 ", " #Src1      \
        ", " #Value1 ")";                                                      \
    const uint32_t T0 = allocateDword();                                       \
    constexpr uint32_t V0 = 0xF00F00;                                          \
    __ mov(IceType_i32, Encoded_GPR_##Src0(), Immediate(Value0));              \
    __ mov(IceType_i32, Encoded_GPR_##Src1(), Immediate(Value1));              \
    __ cmp(IceType_i32, Encoded_GPR_##Src0(), Encoded_GPR_##Src1());           \
    __ mov(IceType_i32, Encoded_GPR_##Dest(), Immediate(0));                   \
    __ setcc(Cond::Br_##C, Encoded_Bytereg_##Dest());                          \
    __ setcc(Cond::Br_##C, dwordAddress(T0));                                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(IsTrue, test.Dest()) << TestString;                              \
    ASSERT_EQ((0xF00F00 | IsTrue), test.contentsOfDword(T0)) << TestString;    \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dest, Src0, Src1)                                             \
  do {                                                                         \
    TestSetCC(o, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(o, Dest, 0u, Src0, 0x1u, Src1, 0x10000000u);                     \
    TestSetCC(no, Dest, 1u, Src0, 0x1u, Src1, 0x10000000u);                    \
    TestSetCC(no, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(b, Dest, 1u, Src0, 0x1, Src1, 0x80000000u);                      \
    TestSetCC(b, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(ae, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(ae, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestSetCC(e, Dest, 1u, Src0, 0x1u, Src1, 0x1u);                            \
    TestSetCC(e, Dest, 0u, Src0, 0x1u, Src1, 0x11111u);                        \
    TestSetCC(ne, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(ne, Dest, 0u, Src0, 0x1u, Src1, 0x1u);                           \
    TestSetCC(be, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestSetCC(be, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(a, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(a, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestSetCC(s, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestSetCC(s, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(ns, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(ns, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestSetCC(p, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(p, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestSetCC(np, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestSetCC(np, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(l, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestSetCC(l, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestSetCC(ge, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestSetCC(ge, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(le, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestSetCC(le, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
  } while (0)

  TestImpl(r1, r2, r3);
  TestImpl(r2, r3, r4);
  TestImpl(r3, r4, r5);
  TestImpl(r4, r5, r6);
  TestImpl(r4, r6, r7);
  TestImpl(r5, r6, r7);
  TestImpl(r6, r7, r8);
  TestImpl(r7, r8, r10);
  TestImpl(r8, r10, r11);
  TestImpl(r10, r11, r12);
  TestImpl(r11, r12, r13);
  TestImpl(r12, r13, r14);
  TestImpl(r13, r14, r15);
  TestImpl(r14, r15, r1);
  TestImpl(r15, r1, r2);

#undef TestImpl
#undef TestSetCC
}

TEST_F(AssemblerX8664Test, Lea) {
#define TestLeaBaseDisp(Base, BaseValue, Disp, Dst)                            \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Base ", " #BaseValue ", " #Dst ")";                               \
    if (Encoded_GPR_##Base() != Encoded_GPR_esp() &&                           \
        Encoded_GPR_##Base() != Encoded_GPR_r9()) {                            \
      __ mov(IceType_i32, Encoded_GPR_##Base(), Immediate(BaseValue));         \
    }                                                                          \
    __ lea(IceType_i32, Encoded_GPR_##Dst(),                                   \
           Address(Encoded_GPR_##Base(), Disp, AssemblerFixup::NoFixup));      \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ(test.Base##d() + (Disp), test.Dst##d())                          \
        << TestString << " with Disp " << Disp;                                \
    reset();                                                                   \
  } while (0)

#define TestLeaIndex32bitDisp(Index, IndexValue, Disp, Dst0, Dst1, Dst2, Dst3) \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Index ", " #IndexValue ", " #Dst0 ", " #Dst1 ", " #Dst2           \
        ", " #Dst3 ")";                                                        \
    if (Encoded_GPR_##Index() != Encoded_GPR_r9()) {                           \
      __ mov(IceType_i32, Encoded_GPR_##Index(), Immediate(IndexValue));       \
    }                                                                          \
    __ lea(IceType_i32, Encoded_GPR_##Dst0(),                                  \
           Address(Encoded_GPR_##Index(), Traits::TIMES_1, Disp,               \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, Encoded_GPR_##Dst1(),                                  \
           Address(Encoded_GPR_##Index(), Traits::TIMES_2, Disp,               \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, Encoded_GPR_##Dst2(),                                  \
           Address(Encoded_GPR_##Index(), Traits::TIMES_4, Disp,               \
                   AssemblerFixup::NoFixup));                                  \
    __ lea(IceType_i32, Encoded_GPR_##Dst3(),                                  \
           Address(Encoded_GPR_##Index(), Traits::TIMES_8, Disp,               \
                   AssemblerFixup::NoFixup));                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((test.Index##d() << Traits::TIMES_1) + (Disp), test.Dst0##d())   \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index##d() << Traits::TIMES_2) + (Disp), test.Dst1##d())   \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index##d() << Traits::TIMES_4) + (Disp), test.Dst2##d())   \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ((test.Index##d() << Traits::TIMES_8) + (Disp), test.Dst3##d())   \
        << TestString << " " << Disp;                                          \
    reset();                                                                   \
  } while (0)

#define TestLeaBaseIndexDisp(Base, BaseValue, Index, IndexValue, Disp, Dst0,   \
                             Dst1, Dst2, Dst3)                                 \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Base ", " #BaseValue ", " #Index ", " #IndexValue ", " #Dst0      \
        ", " #Dst1 ", " #Dst2 ", " #Dst3 ")";                                  \
    if (Encoded_GPR_##Base() != Encoded_GPR_esp() &&                           \
        Encoded_GPR_##Base() != Encoded_GPR_r9()) {                            \
      __ mov(IceType_i32, Encoded_GPR_##Base(), Immediate(BaseValue));         \
    }                                                                          \
                                                                               \
    if (Encoded_GPR_##Index() != Encoded_GPR_r9()) {                           \
      __ mov(IceType_i32, Encoded_GPR_##Index(), Immediate(IndexValue));       \
    }                                                                          \
                                                                               \
    __ lea(IceType_i32, Encoded_GPR_##Dst0(),                                  \
           Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),                \
                   Traits::TIMES_1, Disp, AssemblerFixup::NoFixup));           \
    __ lea(IceType_i32, Encoded_GPR_##Dst1(),                                  \
           Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),                \
                   Traits::TIMES_2, Disp, AssemblerFixup::NoFixup));           \
    __ lea(IceType_i32, Encoded_GPR_##Dst2(),                                  \
           Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),                \
                   Traits::TIMES_4, Disp, AssemblerFixup::NoFixup));           \
    __ lea(IceType_i32, Encoded_GPR_##Dst3(),                                  \
           Address(Encoded_GPR_##Base(), Encoded_GPR_##Index(),                \
                   Traits::TIMES_8, Disp, AssemblerFixup::NoFixup));           \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    uint32_t ExpectedIndexValue = test.Index();                                \
    if (Encoded_GPR_##Index() == Encoded_GPR_esp()) {                          \
      ExpectedIndexValue = 0;                                                  \
    }                                                                          \
    ASSERT_EQ(test.Base##d() + (ExpectedIndexValue << Traits::TIMES_1) +       \
                  (Disp),                                                      \
              test.Dst0##d())                                                  \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base##d() + (ExpectedIndexValue << Traits::TIMES_2) +       \
                  (Disp),                                                      \
              test.Dst1##d())                                                  \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base##d() + (ExpectedIndexValue << Traits::TIMES_4) +       \
                  (Disp),                                                      \
              test.Dst2##d())                                                  \
        << TestString << " " << Disp;                                          \
    ASSERT_EQ(test.Base##d() + (ExpectedIndexValue << Traits::TIMES_8) +       \
                  (Disp),                                                      \
              test.Dst3##d())                                                  \
        << TestString << " " << Disp;                                          \
    reset();                                                                   \
  } while (0)

  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaBaseDisp(r0, 0x22080Fu, Disp, r1);
    TestLeaBaseDisp(r1, 0x10000Fu, Disp, r2);
    TestLeaBaseDisp(r2, 0x20000Fu, Disp, r3);
    TestLeaBaseDisp(r3, 0x30000Fu, Disp, r4);
    TestLeaBaseDisp(r4, 0x40000Fu, Disp, r5);
    TestLeaBaseDisp(r5, 0x50000Fu, Disp, r6);
    TestLeaBaseDisp(r6, 0x60000Fu, Disp, r7);
    TestLeaBaseDisp(r7, 0x11000Fu, Disp, r8);
    TestLeaBaseDisp(r8, 0x11200Fu, Disp, r10);
    TestLeaBaseDisp(r9, 0x220400u, Disp, r10);
    TestLeaBaseDisp(r10, 0x22000Fu, Disp, r11);
    TestLeaBaseDisp(r11, 0x22030Fu, Disp, r12);
    TestLeaBaseDisp(r12, 0x22040Fu, Disp, r13);
    TestLeaBaseDisp(r13, 0x22050Fu, Disp, r14);
    TestLeaBaseDisp(r14, 0x22060Fu, Disp, r15);
    TestLeaBaseDisp(r15, 0x22070Fu, Disp, r1);
  }

  // esp is not a valid index register.
  // ebp is not valid in this addressing mode (rm = 0).
  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaIndex32bitDisp(r1, 0x2000u, Disp, r2, r3, r4, r6);
    TestLeaIndex32bitDisp(r2, 0x4010u, Disp, r3, r4, r6, r7);
    TestLeaIndex32bitDisp(r3, 0x6020u, Disp, r4, r6, r7, r5);
    TestLeaIndex32bitDisp(r4, 0x8030u, Disp, r6, r7, r5, r10);
    TestLeaIndex32bitDisp(r6, 0xA040u, Disp, r7, r5, r10, r1);
    TestLeaIndex32bitDisp(r7, 0xC050u, Disp, r5, r10, r1, r11);
    TestLeaIndex32bitDisp(r8, 0xC060u, Disp, r10, r1, r11, r12);
    TestLeaIndex32bitDisp(r9, 0xC100u, Disp, r1, r11, r12, r13);
    TestLeaIndex32bitDisp(r10, 0xC008u, Disp, r11, r12, r13, r14);
    TestLeaIndex32bitDisp(r11, 0xC009u, Disp, r12, r13, r14, r15);
    TestLeaIndex32bitDisp(r12, 0xC00Au, Disp, r13, r14, r15, r1);
    TestLeaIndex32bitDisp(r13, 0xC00Bu, Disp, r14, r15, r1, r2);
    TestLeaIndex32bitDisp(r14, 0xC00Cu, Disp, r15, r1, r2, r3);
    TestLeaIndex32bitDisp(r15, 0xC00Du, Disp, r1, r2, r3, r4);
  }

  for (const int32_t Disp :
       {0x00, 0x06, -0x06, 0x0600, -0x6000, 0x6000000, -0x6000000}) {
    TestLeaBaseIndexDisp(r1, 0x100000u, r2, 0x600u, Disp, r3, r4, r6, r7);
    TestLeaBaseIndexDisp(r2, 0x200000u, r3, 0x500u, Disp, r4, r6, r7, r8);
    TestLeaBaseIndexDisp(r3, 0x300000u, r4, 0x400u, Disp, r6, r7, r8, r5);
    TestLeaBaseIndexDisp(r4, 0x400000u, r6, 0x300u, Disp, r7, r8, r5, r10);
    TestLeaBaseIndexDisp(r6, 0x500000u, r7, 0x200u, Disp, r8, r5, r10, r11);
    TestLeaBaseIndexDisp(r7, 0x600000u, r8, 0x100u, Disp, r5, r10, r11, r12);
    TestLeaBaseIndexDisp(r8, 0x600000u, r9, 0x1A0u, Disp, r10, r11, r12, r13);
    TestLeaBaseIndexDisp(r9, 0x600050u, r10, 0x1B0u, Disp, r11, r12, r13, r14);
    TestLeaBaseIndexDisp(r10, 0x602000u, r11, 0x1C0u, Disp, r12, r13, r14, r15);
    TestLeaBaseIndexDisp(r11, 0x603000u, r12, 0x1D0u, Disp, r13, r14, r15, r1);
    TestLeaBaseIndexDisp(r12, 0x604000u, r13, 0x1E0u, Disp, r14, r15, r1, r2);
    TestLeaBaseIndexDisp(r13, 0x605000u, r14, 0x1F0u, Disp, r15, r1, r2, r3);
    TestLeaBaseIndexDisp(r14, 0x606000u, r15, 0x10Au, Disp, r1, r2, r3, r4);
    TestLeaBaseIndexDisp(r15, 0x607000u, r1, 0x10Bu, Disp, r2, r3, r4, r6);

    TestLeaBaseIndexDisp(r0, 0, r2, 0x600u, Disp, r3, r4, r6, r7);
    TestLeaBaseIndexDisp(r0, 0, r3, 0x500u, Disp, r4, r6, r7, r8);
    TestLeaBaseIndexDisp(r0, 0, r4, 0x400u, Disp, r6, r7, r8, r5);
    TestLeaBaseIndexDisp(r0, 0, r6, 0x300u, Disp, r7, r8, r5, r10);
    TestLeaBaseIndexDisp(r0, 0, r7, 0x200u, Disp, r8, r5, r10, r11);
    TestLeaBaseIndexDisp(r0, 0, r8, 0x100u, Disp, r5, r10, r11, r12);
    TestLeaBaseIndexDisp(r0, 0, r9, 0x1000u, Disp, r10, r11, r12, r13);
    TestLeaBaseIndexDisp(r0, 0, r10, 0x1B0u, Disp, r11, r12, r13, r14);
    TestLeaBaseIndexDisp(r0, 0, r11, 0x1C0u, Disp, r12, r13, r14, r15);
    TestLeaBaseIndexDisp(r0, 0, r12, 0x1D0u, Disp, r13, r14, r15, r1);
    TestLeaBaseIndexDisp(r0, 0, r13, 0x1E0u, Disp, r14, r15, r1, r2);
    TestLeaBaseIndexDisp(r0, 0, r14, 0x1F0u, Disp, r15, r1, r2, r3);
    TestLeaBaseIndexDisp(r0, 0, r15, 0x10Au, Disp, r1, r2, r3, r4);
    TestLeaBaseIndexDisp(r0, 0, r1, 0x10Bu, Disp, r2, r3, r4, r6);

    TestLeaBaseIndexDisp(r5, 0x100000u, r2, 0x600u, Disp, r3, r4, r6, r7);
    TestLeaBaseIndexDisp(r5, 0x200000u, r3, 0x500u, Disp, r4, r6, r7, r8);
    TestLeaBaseIndexDisp(r5, 0x300000u, r4, 0x400u, Disp, r6, r7, r8, r1);
    TestLeaBaseIndexDisp(r5, 0x400000u, r6, 0x300u, Disp, r7, r8, r1, r10);
    TestLeaBaseIndexDisp(r5, 0x500000u, r7, 0x200u, Disp, r8, r1, r10, r11);
    TestLeaBaseIndexDisp(r5, 0x600000u, r8, 0x100u, Disp, r1, r10, r11, r12);
    TestLeaBaseIndexDisp(r5, 0x600000u, r9, 0x1A00u, Disp, r10, r11, r12, r13);
    TestLeaBaseIndexDisp(r5, 0x601000u, r10, 0x1B0u, Disp, r11, r12, r13, r14);
    TestLeaBaseIndexDisp(r5, 0x602000u, r11, 0x1C0u, Disp, r12, r13, r14, r15);
    TestLeaBaseIndexDisp(r5, 0x603000u, r12, 0x1D0u, Disp, r13, r14, r15, r1);
    TestLeaBaseIndexDisp(r5, 0x604000u, r13, 0x1E0u, Disp, r14, r15, r1, r2);
    TestLeaBaseIndexDisp(r5, 0x605000u, r14, 0x1F0u, Disp, r15, r1, r2, r3);
    TestLeaBaseIndexDisp(r5, 0x606000u, r15, 0x10Au, Disp, r1, r2, r3, r4);
    TestLeaBaseIndexDisp(r5, 0x607000u, r1, 0x10Bu, Disp, r2, r3, r4, r6);

    TestLeaBaseIndexDisp(r2, 0x100000u, r5, 0x600u, Disp, r3, r4, r6, r7);
    TestLeaBaseIndexDisp(r3, 0x200000u, r5, 0x500u, Disp, r4, r6, r7, r8);
    TestLeaBaseIndexDisp(r4, 0x300000u, r5, 0x400u, Disp, r6, r7, r8, r1);
    TestLeaBaseIndexDisp(r6, 0x400000u, r5, 0x300u, Disp, r7, r8, r1, r10);
    TestLeaBaseIndexDisp(r7, 0x500000u, r5, 0x200u, Disp, r8, r1, r10, r11);
    TestLeaBaseIndexDisp(r8, 0x600000u, r5, 0x100u, Disp, r1, r10, r11, r12);
    TestLeaBaseIndexDisp(r9, 0x660000u, r5, 0x1A0u, Disp, r10, r11, r12, r13);
    TestLeaBaseIndexDisp(r10, 0x601000u, r5, 0x1B0u, Disp, r11, r12, r13, r14);
    TestLeaBaseIndexDisp(r11, 0x602000u, r5, 0x1C0u, Disp, r12, r13, r14, r15);
    TestLeaBaseIndexDisp(r12, 0x603000u, r5, 0x1D0u, Disp, r13, r14, r15, r1);
    TestLeaBaseIndexDisp(r13, 0x604000u, r5, 0x1E0u, Disp, r14, r15, r1, r2);
    TestLeaBaseIndexDisp(r14, 0x605000u, r5, 0x1F0u, Disp, r15, r1, r2, r3);
    TestLeaBaseIndexDisp(r15, 0x606000u, r5, 0x10Au, Disp, r1, r2, r3, r4);
    TestLeaBaseIndexDisp(r1, 0x607000u, r5, 0x10Bu, Disp, r2, r3, r4, r6);

    TestLeaBaseIndexDisp(r0, 0, r5, 0xC0BEBEEF, Disp, r2, r3, r4, r6);
  }

// Absolute addressing mode is tested in the Low Level tests. The encoding used
// by the assembler has different meanings in x86-32 and x86-64.
#undef TestLeaBaseIndexDisp
#undef TestLeaScaled32bitDisp
#undef TestLeaBaseDisp
}

TEST_F(AssemblerX8664LowLevelTest, LeaAbsolute) {
#define TestLeaAbsolute(Dst, Value)                                            \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Value ")";             \
    __ lea(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                        \
           Address::Absolute(Value));                                          \
    static constexpr uint32_t ByteCount = 8;                                   \
    ASSERT_EQ(ByteCount, codeBytesSize()) << TestString;                       \
    static constexpr uint8_t Opcode = 0x8D;                                    \
    static constexpr uint8_t ModRM =                                           \
        /*mod*/ 0x00 | /*reg*/ (GPRRegister::Encoded_Reg_##Dst << 3) |         \
        /*rm*/ GPRRegister::Encoded_Reg_esp;                                   \
    static constexpr uint8_t SIB =                                             \
        /*Scale*/ 0x00 | /*Index*/ (GPRRegister::Encoded_Reg_esp << 3) |       \
        /*base*/ GPRRegister::Encoded_Reg_ebp;                                 \
    ASSERT_TRUE(verifyBytes<ByteCount>(                                        \
        codeBytes(), 0x67, Opcode, ModRM, SIB, (Value)&0xFF,                   \
        (Value >> 8) & 0xFF, (Value >> 16) & 0xFF, (Value >> 24) & 0xFF));     \
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

TEST_F(AssemblerX8664Test, Test) {
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Value0));           \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Value1));           \
    __ test(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());        \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(ValueIfFalse));         \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(ValueIfTrue));          \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Value0));           \
    __ test(IceType_i##Size, Encoded_GPR_##Dst(),                              \
            Immediate((Imm)&Mask##Size));                                      \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(ValueIfFalse));         \
    Label Done;                                                                \
    __ j(Cond::Br_e, &Done, NearJump);                                         \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(ValueIfTrue));          \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Value1));           \
    __ test(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src());           \
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

  TestImpl(r1, r2);
  TestImpl(r2, r3);
  TestImpl(r3, r4);
  TestImpl(r4, r5);
  TestImpl(r5, r6);
  TestImpl(r6, r7);
  TestImpl(r7, r8);
  TestImpl(r8, r10);
  TestImpl(r10, r11);
  TestImpl(r11, r12);
  TestImpl(r12, r13);
  TestImpl(r13, r14);
  TestImpl(r14, r15);
  TestImpl(r15, r1);

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
TEST_F(AssemblerX8664Test, Arith_most) {
  static constexpr uint32_t Mask8 = 0xFF;
  static constexpr uint32_t Mask16 = 0xFFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplRegReg(Inst, Dst, Value0, Src, Value1, Type, Size, Op)         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Value0 ", " #Src ", " #Value1                \
        ", " #Type #Size "_t, " #Op ")";                                       \
                                                                               \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Value0));           \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Value1));           \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());        \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Value0));           \
    __ mov(IceType_i##Size, dwordAddress(T0), Immediate(Value1));              \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), dwordAddress(T0));           \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Value0));           \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(),                              \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Value1));           \
    __ Inst(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src());           \
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
    TestImplSize(Dst, Src, 8);                                                 \
    TestImplSize(Dst, Src, 16);                                                \
    TestImplSize(Dst, Src, 32);                                                \
  } while (0)

  TestImpl(r1, r2);
  TestImpl(r2, r3);
  TestImpl(r3, r4);
  TestImpl(r4, r5);
  TestImpl(r5, r6);
  TestImpl(r6, r7);
  TestImpl(r7, r8);
  TestImpl(r8, r10);
  TestImpl(r10, r11);
  TestImpl(r11, r12);
  TestImpl(r12, r13);
  TestImpl(r13, r14);
  TestImpl(r14, r15);
  TestImpl(r15, r1);

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

TEST_F(AssemblerX8664Test, Arith_BorrowNCarry) {
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst0(),                              \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst1(),                              \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ mov(IceType_i##Size, Encoded_GPR_##Src0(),                              \
           Immediate(uint64_t(Value1) & Mask##Size));                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Src1(),                              \
           Immediate((uint64_t(Value1) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, Encoded_GPR_##Dst0(), Encoded_GPR_##Src0());     \
    __ Inst1(IceType_i##Size, Encoded_GPR_##Dst1(), Encoded_GPR_##Src1());     \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst0(),                              \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst1(),                              \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, Encoded_GPR_##Dst0(), dwordAddress(T0));         \
    __ Inst1(IceType_i##Size, Encoded_GPR_##Dst1(), dwordAddress(T1));         \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst0(),                              \
           Immediate(uint64_t(Value0) & Mask##Size));                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst1(),                              \
           Immediate((uint64_t(Value0) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, Encoded_GPR_##Dst0(),                            \
             Immediate(uint64_t(Imm) & Mask##Size));                           \
    __ Inst1(IceType_i##Size, Encoded_GPR_##Dst1(),                            \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Src0(),                              \
           Immediate(uint64_t(Value1) & Mask##Size));                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Src1(),                              \
           Immediate((uint64_t(Value1) >> Size) & Mask##Size));                \
    __ Inst0(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src0());         \
    __ Inst1(IceType_i##Size, dwordAddress(T1), Encoded_GPR_##Src1());         \
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
    TestImplSize(Dst0, Dst1, Src0, Src1, 8);                                   \
    TestImplSize(Dst0, Dst1, Src0, Src1, 16);                                  \
    TestImplSize(Dst0, Dst1, Src0, Src1, 32);                                  \
  } while (0)

  TestImpl(r1, r2, r3, r5);
  TestImpl(r2, r3, r4, r6);
  TestImpl(r3, r4, r5, r7);
  TestImpl(r4, r5, r6, r8);
  TestImpl(r5, r6, r7, r10);
  TestImpl(r6, r7, r8, r11);
  TestImpl(r7, r8, r10, r12);
  TestImpl(r8, r10, r11, r13);
  TestImpl(r10, r11, r12, r14);
  TestImpl(r11, r12, r13, r15);
  TestImpl(r12, r13, r14, r1);
  TestImpl(r13, r14, r15, r2);
  TestImpl(r14, r15, r1, r3);
  TestImpl(r15, r1, r2, r4);

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

TEST_F(AssemblerX8664LowLevelTest, Cbw_Cwd_Cdq) {
#define TestImpl(Inst, BytesSize, ...)                                         \
  do {                                                                         \
    __ Inst();                                                                 \
    ASSERT_EQ(BytesSize, codeBytesSize()) << #Inst;                            \
    ASSERT_TRUE(verifyBytes<BytesSize>(codeBytes(), __VA_ARGS__));             \
    reset();                                                                   \
  } while (0)

  TestImpl(cbw, 2u, 0x66, 0x98);
  TestImpl(cwd, 2u, 0x66, 0x99);
  TestImpl(cdq, 1u, 0x99);

#undef TestImpl
}

TEST_F(AssemblerX8664Test, SingleOperandMul) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplReg(Inst, Value0, Src, Value1, Type, Size)                     \
  do {                                                                         \
    static_assert(Encoded_GPR_eax() != Encoded_GPR_##Src(),                    \
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
    __ mov(IceType_i##Size, Encoded_GPR_eax(),                                 \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, Encoded_GPR_##Src());                             \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_ax(), Immediate(0x00FF));                \
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
    __ mov(IceType_i##Size, Encoded_GPR_eax(),                                 \
           Immediate((Value0)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, dwordAddress(T0));                                \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_ax(), Immediate(0x00FF));                \
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

  TestImpl(r2);
  TestImpl(r3);
  TestImpl(r4);
  TestImpl(r5);
  TestImpl(r6);
  TestImpl(r7);
  TestImpl(r8);
  TestImpl(r10);
  TestImpl(r11);
  TestImpl(r12);
  TestImpl(r13);
  TestImpl(r14);
  TestImpl(r15);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplOp
#undef TestImplAddr
#undef TestImplReg
}

TEST_F(AssemblerX8664Test, TwoOperandImul) {
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ imul(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());        \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_ax(), Immediate(0x00FF));                \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ imul(IceType_i##Size, Encoded_GPR_##Dst(), Immediate(Imm));             \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_ax(), Immediate(0x00FF));                \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ imul(IceType_i##Size, Encoded_GPR_##Dst(), dwordAddress(T0));           \
                                                                               \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_ax(), Immediate(0x00FF));                \
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

  TestImpl(r1, r2);
  TestImpl(r2, r3);
  TestImpl(r3, r4);
  TestImpl(r4, r5);
  TestImpl(r5, r6);
  TestImpl(r6, r7);
  TestImpl(r7, r8);
  TestImpl(r8, r10);
  TestImpl(r10, r11);
  TestImpl(r11, r12);
  TestImpl(r12, r13);
  TestImpl(r13, r14);
  TestImpl(r14, r15);
  TestImpl(r15, r1);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplRegAddr
#undef TestImplRegImm
#undef TestImplRegReg
}

TEST_F(AssemblerX8664Test, Div) {
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
    static_assert(Encoded_GPR_eax() != Encoded_GPR_##Src(),                    \
                  "eax can not be src1.");                                     \
    static_assert(Encoded_GPR_edx() != Encoded_GPR_##Src(),                    \
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
    if (Size == 8) {                                                           \
      /* mov Operand0Hi|Operand0Lo, %ah|%al */                                 \
      __ mov(                                                                  \
          IceType_i16, Encoded_GPR_eax(),                                      \
          Immediate((static_cast<uint16_t>(Operand0Hi) << 8 | Operand0Lo)));   \
    } else {                                                                   \
      __ mov(IceType_i##Size, Encoded_GPR_eax(), Immediate(Operand0Lo));       \
      __ mov(IceType_i##Size, Encoded_GPR_edx(), Immediate(Operand0Hi));       \
    }                                                                          \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Operand1));         \
    __ Inst(IceType_i##Size, Encoded_GPR_##Src());                             \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_eax(), Immediate(0x00FF));               \
      if (Encoded_GPR_##Src() == Encoded_GPR_esi()) {                          \
        __ And(IceType_i16, Encoded_GPR_edx(), Immediate(0x00FF));             \
      }                                                                        \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    static constexpr uint32_t Quocient = (Operand0 / Operand1) & Mask##Size;   \
    static constexpr uint32_t Reminder = (Operand0 % Operand1) & Mask##Size;   \
    ASSERT_EQ(Quocient, test.eax()) << TestString;                             \
    ASSERT_EQ(Reminder, test.edx()) << TestString;                             \
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
    if (Size == 8) {                                                           \
      /* mov Operand0Hi|Operand0Lo, %ah|%al */                                 \
      __ mov(                                                                  \
          IceType_i16, Encoded_GPR_eax(),                                      \
          Immediate((static_cast<uint16_t>(Operand0Hi) << 8 | Operand0Lo)));   \
    } else {                                                                   \
      __ mov(IceType_i##Size, Encoded_GPR_eax(), Immediate(Operand0Lo));       \
      __ mov(IceType_i##Size, Encoded_GPR_edx(), Immediate(Operand0Hi));       \
    }                                                                          \
    __ Inst(IceType_i##Size, dwordAddress(T0));                                \
    if (Size == 8) {                                                           \
      /* mov %ah, %dl */                                                       \
      __ mov(IceType_i16, Encoded_GPR_dx(), Encoded_GPR_ax());                 \
      __ shr(IceType_i32, Encoded_GPR_edx(), Immediate(8));                    \
      __ And(IceType_i16, Encoded_GPR_eax(), Immediate(0x00FF));               \
    }                                                                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, static_cast<uint32_t>(V0));                            \
    test.run();                                                                \
                                                                               \
    static constexpr uint32_t Quocient = (Operand0 / V0) & Mask##Size;         \
    static constexpr uint32_t Reminder = (Operand0 % V0) & Mask##Size;         \
    ASSERT_EQ(Quocient, test.eax()) << TestString;                             \
    ASSERT_EQ(Reminder, test.edx()) << TestString;                             \
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

  TestImpl(r2);
  TestImpl(r3);
  TestImpl(r5);
  TestImpl(r6);
  TestImpl(r7);
  TestImpl(r8);
  TestImpl(r10);
  TestImpl(r11);
  TestImpl(r12);
  TestImpl(r13);
  TestImpl(r14);
  TestImpl(r15);

#undef TestImpl
#undef TestImplSize
#undef TestImplValue
#undef TestImplOp
#undef TestImplAddr
#undef TestImplReg
}

TEST_F(AssemblerX8664Test, Incl_Decl_Addr) {
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

TEST_F(AssemblerX8664Test, Shifts) {
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(),                              \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src(),         \
            Immediate(Count));                                                 \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i8, Encoded_GPR_ecx(), Immediate((Count)&Mask##Size));      \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_ecx());          \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_ecx(), Immediate((Count)&0x7F));       \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());        \
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
    __ mov(IceType_i8, Encoded_GPR_ecx(), Immediate((Count)&Mask##Size));      \
    __ Inst(IceType_i##Size, dwordAddress(T0), Encoded_GPR_ecx());             \
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
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_ecx(), Immediate((Count)&0x7F));       \
    __ Inst(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src());           \
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
    static_assert(Encoded_GPR_##Dst() != Encoded_GPR_ecx(),                    \
                  "ecx should not be specified as Dst");                       \
    TestImplRegImm(Inst, Dst, Value0, Count, Op, Type, Size);                  \
    TestImplRegImm(Inst, ecx, Value0, Count, Op, Type, Size);                  \
    TestImplRegCl(Inst, Dst, Value0, Count, Op, Type, Size);                   \
    TestImplAddrCl(Inst, Value0, Count, Op, Type, Size);                       \
  } while (0)

#define TestImplThreeOperandOp(Inst, Dst, Value0, Src, Value1, Count, Op0,     \
                               Op1, Type, Size)                                \
  do {                                                                         \
    static_assert(Encoded_GPR_##Dst() != Encoded_GPR_ecx(),                    \
                  "ecx should not be specified as Dst");                       \
    static_assert(Encoded_GPR_##Src() != Encoded_GPR_ecx(),                    \
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
    TestImplSize(Dst, 8);                                                      \
    TestImplSize(Dst, 16);                                                     \
    TestImplThreeOperandSize(Dst, Src, 16);                                    \
    TestImplSize(Dst, 32);                                                     \
    TestImplThreeOperandSize(Dst, Src, 32);                                    \
  } while (0)

  TestImpl(r1, r2);
  TestImpl(r2, r4);
  TestImpl(r4, r5);
  TestImpl(r5, r6);
  TestImpl(r6, r7);
  TestImpl(r7, r8);
  TestImpl(r8, r10);
  TestImpl(r10, r11);
  TestImpl(r11, r12);
  TestImpl(r12, r13);
  TestImpl(r13, r14);
  TestImpl(r14, r15);
  TestImpl(r15, r1);

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

TEST_F(AssemblerX8664Test, Neg) {
  static constexpr uint32_t Mask8 = 0x000000ff;
  static constexpr uint32_t Mask16 = 0x0000ffff;
  static constexpr uint32_t Mask32 = 0xffffffff;

#define TestImplReg(Dst, Size)                                                 \
  do {                                                                         \
    static constexpr int32_t Value = 0xFF00A543;                               \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(),                               \
           Immediate(static_cast<int##Size##_t>(Value) & Mask##Size));         \
    __ neg(IceType_i##Size, Encoded_GPR_##Dst());                              \
    __ mov(IceType_i##Size, Encoded_GPR_eax(), Encoded_GPR_##Dst());           \
    __ And(IceType_i32, Encoded_GPR_eax(), Immediate(Mask##Size));             \
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
    TestImplReg(r1, Size);                                                     \
    TestImplReg(r2, Size);                                                     \
    TestImplReg(r3, Size);                                                     \
    TestImplReg(r4, Size);                                                     \
    TestImplReg(r5, Size);                                                     \
    TestImplReg(r6, Size);                                                     \
    TestImplReg(r7, Size);                                                     \
    TestImplReg(r8, Size);                                                     \
    TestImplReg(r10, Size);                                                    \
    TestImplReg(r11, Size);                                                    \
    TestImplReg(r12, Size);                                                    \
    TestImplReg(r13, Size);                                                    \
    TestImplReg(r14, Size);                                                    \
    TestImplReg(r15, Size);                                                    \
  } while (0)

  TestImpl(8);
  TestImpl(16);
  TestImpl(32);

#undef TestImpl
#undef TestImplAddr
#undef TestImplReg
}

TEST_F(AssemblerX8664Test, Not) {
#define TestImpl(Dst)                                                          \
  do {                                                                         \
    static constexpr uint32_t Value = 0xFF00A543;                              \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(Value));                \
    __ notl(Encoded_GPR_##Dst());                                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(~Value, test.Dst()) << "(" #Dst ")";                             \
    reset();                                                                   \
  } while (0)

  TestImpl(r1);
  TestImpl(r2);
  TestImpl(r3);
  TestImpl(r4);
  TestImpl(r5);
  TestImpl(r6);
  TestImpl(r7);
  TestImpl(r8);
  TestImpl(r10);
  TestImpl(r11);
  TestImpl(r12);
  TestImpl(r13);
  TestImpl(r14);
  TestImpl(r15);

#undef TestImpl
}

TEST_F(AssemblerX8664Test, Bswap) {
#define TestImpl(Dst)                                                          \
  do {                                                                         \
    static constexpr uint32_t Value = 0xFF00A543;                              \
    static constexpr uint32_t Expected = 0x43A500FF;                           \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(Value));                \
    __ bswap(IceType_i32, Encoded_GPR_##Dst());                                \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst()) << "(" #Dst ")";                           \
    reset();                                                                   \
  } while (0)

  TestImpl(r1);
  TestImpl(r2);
  TestImpl(r3);
  TestImpl(r4);
  TestImpl(r5);
  TestImpl(r6);
  TestImpl(r7);
  TestImpl(r8);
  TestImpl(r10);
  TestImpl(r11);
  TestImpl(r12);
  TestImpl(r13);
  TestImpl(r14);
  TestImpl(r15);

#undef TestImpl
}

TEST_F(AssemblerX8664Test, Bt) {
#define TestImpl(Dst, Value0, Src, Value1)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ")";                      \
    static constexpr uint32_t Expected = ((Value0) & (1u << (Value1))) != 0;   \
                                                                               \
    __ mov(IceType_i32, Encoded_GPR_##Dst(), Immediate(Value0));               \
    __ mov(IceType_i32, Encoded_GPR_##Src(), Immediate(Value1));               \
    __ bt(Encoded_GPR_##Dst(), Encoded_GPR_##Src());                           \
    __ setcc(Cond::Br_b, ByteRegister::Encoded_8_Reg_al);                      \
    __ And(IceType_i32, Encoded_GPR_eax(), Immediate(0xFFu));                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.eax()) << TestString;                             \
    reset();                                                                   \
  } while (0)

  TestImpl(r1, 0x08000000, r2, 27u);
  TestImpl(r2, 0x08000000, r3, 23u);
  TestImpl(r3, 0x00000000, r4, 1u);
  TestImpl(r4, 0x08000300, r5, 9u);
  TestImpl(r5, 0x08000300, r6, 10u);
  TestImpl(r6, 0x7FFFEFFF, r7, 13u);
  TestImpl(r7, 0x08000000, r8, 27u);
  TestImpl(r8, 0x08000000, r10, 23u);
  TestImpl(r10, 0x00000000, r11, 1u);
  TestImpl(r11, 0x08000300, r12, 9u);
  TestImpl(r12, 0x08000300, r13, 10u);
  TestImpl(r13, 0x7FFFEFFF, r14, 13u);
  TestImpl(r14, 0x08000000, r15, 27u);
  TestImpl(r15, 0x08000000, r1, 23u);

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

TEST_F(AssemblerX8664Test, BitScanOperations) {
#define TestImplRegReg(Inst, Dst, Src, Value1, Size)                           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Inst ", " #Dst ", " #Src ", " #Value1 ", " #Size ")";             \
    static constexpr uint32_t Expected = BitScanHelper<Value1, Size>::Inst;    \
    const uint32_t ZeroFlag = allocateDword();                                 \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(), Immediate(Value1));           \
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());        \
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
    __ Inst(IceType_i##Size, Encoded_GPR_##Dst(), dwordAddress(T0));           \
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

  TestImpl(r1, r2);
  TestImpl(r2, r3);
  TestImpl(r3, r4);
  TestImpl(r4, r5);
  TestImpl(r5, r6);
  TestImpl(r6, r7);
  TestImpl(r7, r8);
  TestImpl(r8, r10);
  TestImpl(r10, r11);
  TestImpl(r11, r12);
  TestImpl(r12, r13);
  TestImpl(r13, r14);
  TestImpl(r14, r15);
  TestImpl(r15, r1);

#undef TestImpl
#undef TestImplValue
#undef TestImplSize
#undef TestImplRegAddr
#undef TestImplRegReg
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
