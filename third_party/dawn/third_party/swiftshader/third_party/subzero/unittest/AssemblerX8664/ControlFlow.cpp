//===- subzero/unittest/AssemblerX8664/ControlFlow.cpp --------------------===//
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

TEST_F(AssemblerX8664Test, J) {
#define TestJ(C, Near, Dest, Src0, Value0, Src1, Value1)                       \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #C ", " #Near ", " #Dest ", " #Src0 ", " #Value0 ", " #Src1        \
        ", " #Value1 ")";                                                      \
    const bool NearJmp = AssemblerX8664::k##Near##Jump;                        \
    Label ShouldBeTaken;                                                       \
    __ mov(IceType_i32, Encoded_GPR_##Src0(), Immediate(Value0));              \
    __ mov(IceType_i32, Encoded_GPR_##Src1(), Immediate(Value1));              \
    __ mov(IceType_i32, Encoded_GPR_##Dest(), Immediate(0xBEEF));              \
    __ cmp(IceType_i32, Encoded_GPR_##Src0(), Encoded_GPR_##Src1());           \
    __ j(Cond::Br_##C, &ShouldBeTaken, NearJmp);                               \
    __ mov(IceType_i32, Encoded_GPR_##Dest(), Immediate(0xC0FFEE));            \
    __ bind(&ShouldBeTaken);                                                   \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ(Value0, test.Src0()) << TestString;                              \
    ASSERT_EQ(Value1, test.Src1()) << TestString;                              \
    ASSERT_EQ(0xBEEFul, test.Dest()) << TestString;                            \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src0, Src1)                                              \
  do {                                                                         \
    TestJ(o, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(o, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                       \
    TestJ(no, Near, Dst, Src0, 0x1ul, Src1, 0x1ul);                            \
    TestJ(no, Far, Dst, Src0, 0x1ul, Src1, 0x1ul);                             \
    TestJ(b, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(b, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                       \
    TestJ(ae, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                     \
    TestJ(ae, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(e, Near, Dst, Src0, 0x80000000ul, Src1, 0x80000000ul);               \
    TestJ(e, Far, Dst, Src0, 0x80000000ul, Src1, 0x80000000ul);                \
    TestJ(ne, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                     \
    TestJ(ne, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(be, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                     \
    TestJ(be, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(a, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(a, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                       \
    TestJ(s, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(s, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                       \
    TestJ(ns, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                     \
    TestJ(ns, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(p, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(p, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                       \
    TestJ(np, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                     \
    TestJ(np, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(l, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(l, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                       \
    TestJ(ge, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                     \
    TestJ(ge, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(le, Near, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                     \
    TestJ(le, Far, Dst, Src0, 0x80000000ul, Src1, 0x1ul);                      \
    TestJ(g, Near, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                      \
    TestJ(g, Far, Dst, Src0, 0x1ul, Src1, 0x80000000ul);                       \
  } while (0)

  TestImpl(r1, r2, r3);
  TestImpl(r2, r3, r4);
  TestImpl(r3, r4, r5);
  TestImpl(r4, r5, r6);
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
#undef TestJ
}

TEST_F(AssemblerX8664Test, CallImm) {
  __ call(Immediate(16));
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0xf00f));
  __ popl(GPRRegister::Encoded_Reg_ebx);

  AssembledTest test = assemble();

  test.run();

  EXPECT_EQ(0xF00Fu, test.eax());
}

TEST_F(AssemblerX8664Test, CallReg) {
#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    __ call(Immediate(16));                                                    \
    int CallTargetAddr = codeBytesSize() + 12;                                 \
    __ popl(Encoded_GPR_##Dst());                                              \
    __ pushl(Encoded_GPR_##Dst());                                             \
    __ ret();                                                                  \
    for (int I = codeBytesSize(); I < CallTargetAddr; ++I) {                   \
      __ hlt();                                                                \
    }                                                                          \
    __ popl(Encoded_GPR_##Src());                                              \
    __ call(Encoded_GPR_##Src());                                              \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_LE(15u, test.Dst() - test.Src()) << "(" #Dst ", " #Src ")";         \
    reset();                                                                   \
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
}

TEST_F(AssemblerX8664Test, CallAddr) {
#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = 0xA0C0FFEEBEEFFEEFull;                                 \
    const uint32_t T1 = allocateDword();                                       \
    __ call(Immediate(16));                                                    \
    int CallTargetAddr = codeBytesSize() + 12;                                 \
    __ mov(IceType_i8, Encoded_GPR_##Dst##l(), Immediate(0xf4));               \
    __ ret();                                                                  \
    for (int I = codeBytesSize(); I < CallTargetAddr; ++I) {                   \
      __ hlt();                                                                \
    }                                                                          \
    __ mov(IceType_i64, Encoded_GPR_##Dst##q(), dwordAddress(T0));             \
    __ popl(Encoded_GPR_##Src##q());                                           \
    __ mov(IceType_i32, dwordAddress(T1), Encoded_GPR_##Src##d());             \
    __ call(dwordAddress(T1));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(0xA0C0FFEEBEEFFEF4ull, test.Dst##q()) << "(" #Dst ", " #Src ")"; \
    reset();                                                                   \
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
}

TEST_F(AssemblerX8664Test, Jmp) {
// TestImplReg uses jmp(Label), so jmp(Label) needs to be tested before it.
#define TestImplAddr(Near)                                                     \
  do {                                                                         \
    Label ForwardJmp;                                                          \
    Label BackwardJmp;                                                         \
    Label Done;                                                                \
                                                                               \
    __ jmp(&ForwardJmp, AssemblerX8664::k##Near##Jump);                        \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ bind(&BackwardJmp);                                                     \
    __ jmp(&Done, AssemblerX8664::k##Near##Jump);                              \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ bind(&ForwardJmp);                                                      \
    __ jmp(&BackwardJmp, AssemblerX8664::k##NearJump);                         \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ bind(&Done);                                                            \
  } while (0)

#define TestImplReg(Dst)                                                       \
  do {                                                                         \
    __ call(Immediate(16));                                                    \
    Label Done;                                                                \
    __ jmp(&Done, AssemblerX8664::kNearJump);                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ popl(Encoded_GPR_##Dst());                                              \
    __ jmp(Encoded_GPR_##Dst());                                               \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ hlt();                                                                  \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestImplAddr(Near);
  TestImplAddr(Far);

  TestImplReg(r1);
  TestImplReg(r2);
  TestImplReg(r3);
  TestImplReg(r4);
  TestImplReg(r5);
  TestImplReg(r6);
  TestImplReg(r7);
  TestImplReg(r8);
  TestImplReg(r10);
  TestImplReg(r11);
  TestImplReg(r12);
  TestImplReg(r13);
  TestImplReg(r14);
  TestImplReg(r15);

#undef TestImplReg
#undef TestImplAddr
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
