//===- subzero/unittest/AssemblerX8632/ControleFlow.cpp -------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AssemblerX8632/TestUtil.h"
#include "IceAssemblerX8632.h"

namespace Ice {
namespace X8632 {
namespace Test {
namespace {

TEST_F(AssemblerX8632Test, J) {
#define TestJ(C, Near, Src0, Value0, Src1, Value1, Dest)                       \
  do {                                                                         \
    const bool NearJmp = AssemblerX8632::k##Near##Jump;                        \
    Label ShouldBeTaken;                                                       \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src0, Immediate(Value0));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src1, Immediate(Value1));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dest, Immediate(0xBEEF));   \
    __ cmp(IceType_i32, GPRRegister::Encoded_Reg_##Src0,                       \
           GPRRegister::Encoded_Reg_##Src1);                                   \
    __ j(Cond::Br_##C, &ShouldBeTaken, NearJmp);                               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dest, Immediate(0xC0FFEE)); \
    __ bind(&ShouldBeTaken);                                                   \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    EXPECT_EQ(Value0, test.Src0()) << "Br_" #C ", " #Near;                     \
    EXPECT_EQ(Value1, test.Src1()) << "Br_" #C ", " #Near;                     \
    EXPECT_EQ(0xBEEFul, test.Dest()) << "Br_" #C ", " #Near;                   \
    reset();                                                                   \
  } while (0)

  TestJ(o, Near, eax, 0x80000000ul, ebx, 0x1ul, ecx);
  TestJ(o, Far, ebx, 0x80000000ul, ecx, 0x1ul, edx);
  TestJ(no, Near, ecx, 0x1ul, edx, 0x1ul, edi);
  TestJ(no, Far, edx, 0x1ul, edi, 0x1ul, esi);
  TestJ(b, Near, edi, 0x1ul, esi, 0x80000000ul, eax);
  TestJ(b, Far, esi, 0x1ul, eax, 0x80000000ul, ebx);
  TestJ(ae, Near, eax, 0x80000000ul, ebx, 0x1ul, ecx);
  TestJ(ae, Far, ebx, 0x80000000ul, ecx, 0x1ul, edx);
  TestJ(e, Near, ecx, 0x80000000ul, edx, 0x80000000ul, edi);
  TestJ(e, Far, edx, 0x80000000ul, edi, 0x80000000ul, esi);
  TestJ(ne, Near, edi, 0x80000000ul, esi, 0x1ul, eax);
  TestJ(ne, Far, esi, 0x80000000ul, eax, 0x1ul, ebx);
  TestJ(be, Near, eax, 0x1ul, ebx, 0x80000000ul, ecx);
  TestJ(be, Far, ebx, 0x1ul, ecx, 0x80000000ul, edx);
  TestJ(a, Near, ecx, 0x80000000ul, edx, 0x1ul, edi);
  TestJ(a, Far, edx, 0x80000000ul, edi, 0x1ul, esi);
  TestJ(s, Near, edi, 0x1ul, esi, 0x80000000ul, eax);
  TestJ(s, Far, esi, 0x1ul, eax, 0x80000000ul, ebx);
  TestJ(ns, Near, eax, 0x80000000ul, ebx, 0x1ul, ecx);
  TestJ(ns, Far, ebx, 0x80000000ul, ecx, 0x1ul, edx);
  TestJ(p, Near, ecx, 0x80000000ul, edx, 0x1ul, edi);
  TestJ(p, Far, edx, 0x80000000ul, edi, 0x1ul, esi);
  TestJ(np, Near, edi, 0x1ul, esi, 0x80000000ul, eax);
  TestJ(np, Far, esi, 0x1ul, eax, 0x80000000ul, ebx);
  TestJ(l, Near, eax, 0x80000000ul, ebx, 0x1ul, ecx);
  TestJ(l, Far, ebx, 0x80000000ul, ecx, 0x1ul, edx);
  TestJ(ge, Near, ecx, 0x1ul, edx, 0x80000000ul, edi);
  TestJ(ge, Far, edx, 0x1ul, edi, 0x80000000ul, esi);
  TestJ(le, Near, edi, 0x80000000ul, esi, 0x1ul, eax);
  TestJ(le, Far, esi, 0x80000000ul, eax, 0x1ul, ebx);
  TestJ(g, Near, eax, 0x1ul, ebx, 0x80000000ul, ecx);
  TestJ(g, Far, ebx, 0x1ul, ecx, 0x80000000ul, edx);

#undef TestJ
}

TEST_F(AssemblerX8632Test, CallImm) {
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

TEST_F(AssemblerX8632Test, CallReg) {
  __ call(Immediate(16));
  __ popl(GPRRegister::Encoded_Reg_edx);
  __ pushl(GPRRegister::Encoded_Reg_edx);
  __ ret();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ popl(GPRRegister::Encoded_Reg_ebx);
  __ call(GPRRegister::Encoded_Reg_ebx);

  AssembledTest test = assemble();

  test.run();

  EXPECT_EQ(15u, test.edx() - test.ebx());
}

TEST_F(AssemblerX8632Test, CallAddr) {
  __ call(Immediate(16));
  __ mov(IceType_i8, GPRRegister::Encoded_Reg_eax, Immediate(0xf4));
  __ ret();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ hlt();
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0xf1f2f300));
  __ call(Address(GPRRegister::Encoded_Reg_esp, 0, AssemblerFixup::NoFixup));
  __ popl(GPRRegister::Encoded_Reg_edx);

  AssembledTest test = assemble();

  test.run();

  EXPECT_EQ(0xf1f2f3f4, test.eax());
}

TEST_F(AssemblerX8632Test, Jmp) {
// TestImplReg uses jmp(Label), so jmp(Label) needs to be tested before it.
#define TestImplAddr(Near)                                                     \
  do {                                                                         \
    Label ForwardJmp;                                                          \
    Label BackwardJmp;                                                         \
    Label Done;                                                                \
                                                                               \
    __ jmp(&ForwardJmp, AssemblerX8632::k##Near##Jump);                        \
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
    __ jmp(&Done, AssemblerX8632::k##Near##Jump);                              \
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
    __ jmp(&BackwardJmp, AssemblerX8632::k##NearJump);                         \
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
    __ jmp(&Done, AssemblerX8632::kNearJump);                                  \
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
    __ popl(GPRRegister::Encoded_Reg_##Dst);                                   \
    __ jmp(GPRRegister::Encoded_Reg_##Dst);                                    \
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

  TestImplReg(eax);
  TestImplReg(ebx);
  TestImplReg(ecx);
  TestImplReg(edx);
  TestImplReg(esi);
  TestImplReg(edi);

#undef TestImplReg
#undef TestImplAddr
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice
