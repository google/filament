//===- subzero/unittest/AssemblerX8664/DataMov.cpp ------------------------===//
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

TEST_F(AssemblerX8664Test, MovRegImm) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define MovRegImm(Reg, Suffix, Size)                                           \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Reg ", " #Size ")";              \
    static constexpr uint32_t Value = (0xABCD7645) & Mask##Size;               \
    static constexpr uint32_t Marker = 0xBEEFFEEB;                             \
    __ mov(IceType_i32, Encoded_GPR_##Reg##q(), Immediate(Marker));            \
    __ mov(IceType_i##Size, Encoded_GPR_##Reg##Suffix(), Immediate(Value));    \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Reg##Suffix()) << TestString;                        \
    ASSERT_EQ((Marker & ~Mask##Size) | Value, test.Reg##d()) << TestString;    \
    reset();                                                                   \
  } while (0)

#define TestImpl(Reg)                                                          \
  do {                                                                         \
    MovRegImm(Reg, l, 8);                                                      \
    MovRegImm(Reg, w, 16);                                                     \
    MovRegImm(Reg, d, 32);                                                     \
    /* MovRegImm64 not implemented */                                          \
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
#undef MovRegImm
}

TEST_F(AssemblerX8664Test, MovMemImm) {
  const uint32_t T0 = allocateDword();
  constexpr uint32_t ExpectedT0 = 0x00111100ul;
  const uint32_t T1 = allocateDword();
  constexpr uint32_t ExpectedT1 = 0x00222200ul;
  const uint32_t T2 = allocateDword();
  constexpr uint32_t ExpectedT2 = 0x03333000ul;
  const uint32_t T3 = allocateDword();
  constexpr uint32_t ExpectedT3 = 0x00444400ul;

  __ mov(IceType_i32, dwordAddress(T0), Immediate(ExpectedT0));
  __ mov(IceType_i16, dwordAddress(T1), Immediate(ExpectedT1));
  __ mov(IceType_i8, dwordAddress(T2), Immediate(ExpectedT2));
  __ mov(IceType_i32, dwordAddress(T3), Immediate(ExpectedT3));

  AssembledTest test = assemble();
  test.run();
  EXPECT_EQ(0ul, test.eax());
  EXPECT_EQ(0ul, test.ebx());
  EXPECT_EQ(0ul, test.ecx());
  EXPECT_EQ(0ul, test.edx());
  EXPECT_EQ(0ul, test.edi());
  EXPECT_EQ(0ul, test.esi());
  EXPECT_EQ(ExpectedT0, test.contentsOfDword(T0));
  EXPECT_EQ(ExpectedT1 & 0xFFFF, test.contentsOfDword(T1));
  EXPECT_EQ(ExpectedT2 & 0xFF, test.contentsOfDword(T2));
  EXPECT_EQ(ExpectedT3, test.contentsOfDword(T3));
}

TEST_F(AssemblerX8664Test, MovMemReg) {
  static constexpr uint64_t Mask8 = 0x00000000000000FF;
  static constexpr uint64_t Mask16 = 0x000000000000FFFF;
  static constexpr uint64_t Mask32 = 0x00000000FFFFFFFF;
  static constexpr uint64_t Mask64 = 0xFFFFFFFFFFFFFFFF;

#define TestMemReg(Src, Size)                                                  \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ", " #Size ")";              \
    static constexpr uint32_t Value = 0x1a4d567e & Mask##Size;                 \
    static constexpr uint64_t Marker = 0xD0DA33EEBEEFFEEB;                     \
    const uint32_t T0 = allocateQword();                                       \
                                                                               \
    __ mov(IceType_i32, Encoded_GPR_##Src(), Immediate(Value));                \
    __ mov(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src());            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, Marker);                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((Marker & ~Mask##Size) | Value, test.contentsOfQword(T0))        \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImpl(Src)                                                          \
  do {                                                                         \
    TestMemReg(Src, 8);                                                        \
    TestMemReg(Src, 16);                                                       \
    TestMemReg(Src, 32);                                                       \
    TestMemReg(Src, 64);                                                       \
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
#undef TestMemReg
}

TEST_F(AssemblerX8664Test, MovRegReg) {
  static constexpr uint64_t Mask8 = 0x00000000000000FFull;
  static constexpr uint64_t Mask16 = 0x000000000000FFFFull;
  static constexpr uint64_t Mask32 = 0x00000000FFFFFFFFull;
  static constexpr uint64_t Mask64 = 0xFFFFFFFFFFFFFFFFull;

  static constexpr uint64_t MaskResult8 = 0x00000000000000FFull;
  static constexpr uint64_t MaskResult16 = 0x000000000000FFFFull;
  static constexpr uint64_t MaskResult32 = 0xFFFFFFFFFFFFFFFFull;
  static constexpr uint64_t MaskResult64 = 0xFFFFFFFFFFFFFFFFull;

#define TestRegReg(Dst, Src, Suffix, Size)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Suffix ", " #Size ")";                        \
    const uint8_t T0 = allocateQword();                                        \
    static constexpr uint64_t Value = 0xA4DD30Af86CCE321ull & Mask##Size;      \
    const uint8_t T1 = allocateQword();                                        \
    static constexpr uint64_t Marker = 0xC0FFEEA0BEEFFEEFull;                  \
                                                                               \
    __ mov(IceType_i64, Encoded_GPR_##Src(), dwordAddress(T0));                \
    __ mov(IceType_i64, Encoded_GPR_##Dst(), dwordAddress(T1));                \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), Encoded_GPR_##Src());         \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, Value);                                                \
    test.setQwordTo(T1, Marker);                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((Marker & ~MaskResult##Size) | Value, test.Dst()) << TestString; \
    ASSERT_EQ(Value, test.Dst##Suffix()) << TestString;                        \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestRegReg(Dst, Src, l, 8);                                                \
    TestRegReg(Dst, Src, w, 16);                                               \
    TestRegReg(Dst, Src, d, 32);                                               \
    TestRegReg(Dst, Src, q, 64);                                               \
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
#undef TestRegReg
}

TEST_F(AssemblerX8664Test, MovRegMem) {
  static constexpr uint64_t Mask8 = 0x00000000000000FFull;
  static constexpr uint64_t Mask16 = 0x000000000000FFFFull;
  static constexpr uint64_t Mask32 = 0x00000000FFFFFFFFull;
  static constexpr uint64_t Mask64 = 0xFFFFFFFFFFFFFFFFull;

  static constexpr uint64_t MaskResult8 = ~0x00000000000000FFull;
  static constexpr uint64_t MaskResult16 = ~0x000000000000FFFFull;
  static constexpr uint64_t MaskResult32 = ~0xFFFFFFFFFFFFFFFFull;
  static constexpr uint64_t MaskResult64 = ~0xFFFFFFFFFFFFFFFFull;

#define TestRegAddr(Dst, Suffix, Size)                                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, " #Suffix ", " #Size ")";                            \
    const uint8_t T0 = allocateQword();                                        \
    static constexpr uint64_t Value = 0xA4DD30Af86CCE321ull & Mask##Size;      \
    const uint8_t T1 = allocateQword();                                        \
    static constexpr uint64_t Marker = 0xC0FFEEA0BEEFFEEFull;                  \
                                                                               \
    __ mov(IceType_i64, Encoded_GPR_##Dst(), dwordAddress(T1));                \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst(), dwordAddress(T0));            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, Value);                                                \
    test.setQwordTo(T1, Marker);                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((Marker & MaskResult##Size) | Value, test.Dst()) << TestString;  \
    ASSERT_EQ(Value, test.Dst##Suffix()) << TestString;                        \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst)                                                          \
  do {                                                                         \
    TestRegAddr(Dst, l, 8);                                                    \
    TestRegAddr(Dst, w, 16);                                                   \
    TestRegAddr(Dst, d, 32);                                                   \
    TestRegAddr(Dst, q, 64);                                                   \
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
#undef TestRegAddr
}

TEST_F(AssemblerX8664Test, Movabs) {
#define TestImplValue(Dst, Value)                                              \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Value ")";             \
    uint64_t V = (Value);                                                      \
    __ movabs(Encoded_GPR_##Dst##q(), V);                                      \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V, test.DST()) << TestString;                                    \
  } while (0)

#define TestImpl(Dst)                                                          \
  do {                                                                         \
    for (uint64_t V = {0, 1, 0xFFFFFFull, 0x80000000ull,                       \
                       0xFFFFFFFFFFFFFFFFull}) {                               \
      TestImpl(Dst, V);                                                        \
    }                                                                          \
  } while (0)

#undef TestImpl
#undef TestImplValue
}

TEST_F(AssemblerX8664Test, Movzx) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;

#define TestImplRegReg(Dst, Src, Suffix, Size)                                 \
  do {                                                                         \
    const uint32_t T0 = allocateDqword();                                      \
    static constexpr uint64_t V0 = 0xAAAAAAAAAAAAAAAAull;                      \
    static constexpr uint32_t Value = (0xBEEF) & Mask##Size;                   \
    __ mov(IceType_i64, Encoded_GPR_##Dst##q(), dwordAddress(T0));             \
    __ mov(IceType_i##Size, Encoded_GPR_##Src##Suffix(), Immediate(Value));    \
    __ movzx(IceType_i##Size, Encoded_GPR_##Dst##d(),                          \
             Encoded_GPR_##Src##Suffix());                                     \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ(Value, test.Dst##q()) << "(" #Dst ", " #Src ", " #Size ")";      \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Dst, Suffix, Size)                                     \
  do {                                                                         \
    const uint32_t T0 = allocateDqword();                                      \
    static constexpr uint64_t V0 = 0xAAAAAAAAAAAAAAAAull;                      \
    static constexpr uint32_t Value = (0xBEEF) & Mask##Size;                   \
    __ movzx(IceType_i##Size, Encoded_GPR_##Dst##d(), dwordAddress(T0));       \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, (V0 & ~Mask##Size) | Value);                           \
    test.run();                                                                \
    ASSERT_EQ(Value, test.Dst##q()) << "(" #Dst ", Addr, " #Size ")";          \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplRegReg(Dst, Src, l, 8);                                            \
    TestImplRegAddr(Dst, l, 8);                                                \
    TestImplRegReg(Dst, Src, w, 16);                                           \
    TestImplRegAddr(Dst, w, 16);                                               \
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
#undef TestImplRegAddr
#undef TestImplRegReg
}

TEST_F(AssemblerX8664Test, Movsx) {
  static constexpr uint64_t Mask8 = 0x000000FF;
  static constexpr uint64_t Mask16 = 0x0000FFFF;
  static constexpr uint64_t Mask32 = 0xFFFFFFFF;

#define TestImplRegReg(Dst, Src, Suffix, Size)                                 \
  do {                                                                         \
    const uint32_t T0 = allocateDqword();                                      \
    static constexpr uint64_t V0 = 0xAAAAAAAAAAAAAAAAull;                      \
    static constexpr uint64_t Value = (0xC0BEBEEF) & Mask##Size;               \
    __ mov(IceType_i64, Encoded_GPR_##Dst##q(), dwordAddress(T0));             \
    __ mov(IceType_i##Size, Encoded_GPR_##Src##Suffix(), Immediate(Value));    \
    __ movsx(IceType_i##Size, Encoded_GPR_##Dst##d(),                          \
             Encoded_GPR_##Src##Suffix());                                     \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((uint64_t(-1) & ~Mask##Size) | Value, test.Dst##q())             \
        << "(" #Dst ", " #Src ", " #Size ")";                                  \
    reset();                                                                   \
  } while (0)

#define TestImplRegAddr(Dst, Suffix, Size)                                     \
  do {                                                                         \
    const uint32_t T0 = allocateDqword();                                      \
    static constexpr uint64_t V0 = 0xC0BEBEEF & Mask##Size;                    \
    static constexpr uint64_t Value = (0xC0BEBEEF) & Mask##Size;               \
    __ movsx(IceType_i##Size, Encoded_GPR_##Dst##d(), dwordAddress(T0));       \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((uint64_t(-1) & ~Mask##Size) | Value, test.Dst##q())             \
        << "(" #Dst ", Addr, " #Size ")";                                      \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplRegReg(Dst, Src, l, 8);                                            \
    TestImplRegAddr(Dst, l, 8);                                                \
    TestImplRegReg(Dst, Src, w, 16);                                           \
    TestImplRegAddr(Dst, w, 16);                                               \
    TestImplRegReg(Dst, Src, w, 32);                                           \
    TestImplRegAddr(Dst, w, 32);                                               \
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
#undef TestImplRegAddr
#undef TestImplRegReg
}

TEST_F(AssemblerX8664Test, Cmov) {
#define TestRegReg(C, Dest, IsTrue, Src0, Value0, Src1, Value1)                \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #C ", " #Dest ", " #IsTrue ", " #Src0 ", " #Value0 ", " #Src1      \
        ", " #Value1 ")";                                                      \
    __ mov(IceType_i32, Encoded_GPR_##Src0(), Immediate(Value0));              \
    __ mov(IceType_i32, Encoded_GPR_##Src1(), Immediate(Value1));              \
    __ mov(IceType_i32, Encoded_GPR_##Dest(), Immediate(Value0));              \
    __ cmp(IceType_i32, Encoded_GPR_##Src0(), Encoded_GPR_##Src1());           \
    __ cmov(IceType_i32, Cond::Br_##C, Encoded_GPR_##Dest(),                   \
            Encoded_GPR_##Src1());                                             \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((IsTrue) ? (Value1) : (Value0), test.Dest()) << TestString;      \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestRegAddr(C, Dest, IsTrue, Src0, Value0, Value1)                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #C ", " #Dest ", " #IsTrue ", " #Src0 ", " #Value0                 \
        ", Addr, " #Value1 ")";                                                \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value1;                                                \
    __ mov(IceType_i32, Encoded_GPR_##Src0(), Immediate(Value0));              \
    __ mov(IceType_i32, Encoded_GPR_##Dest(), Immediate(Value0));              \
    __ cmp(IceType_i32, Encoded_GPR_##Src0(), dwordAddress(T0));               \
    __ cmov(IceType_i32, Cond::Br_##C, Encoded_GPR_##Dest(),                   \
            dwordAddress(T0));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((IsTrue) ? (Value1) : (Value0), test.Dest()) << TestString;      \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestValue(C, Dest, IsTrue, Src0, Value0, Src1, Value1)                 \
  do {                                                                         \
    TestRegReg(C, Dest, IsTrue, Src0, Value0, Src1, Value1);                   \
    TestRegAddr(C, Dest, IsTrue, Src0, Value0, Value1);                        \
  } while (0)

#define TestImpl(Dest, Src0, Src1)                                             \
  do {                                                                         \
    TestValue(o, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(o, Dest, 0u, Src0, 0x1u, Src1, 0x10000000u);                     \
    TestValue(no, Dest, 1u, Src0, 0x1u, Src1, 0x10000000u);                    \
    TestValue(no, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(b, Dest, 1u, Src0, 0x1, Src1, 0x80000000u);                      \
    TestValue(b, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(ae, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(ae, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestValue(e, Dest, 1u, Src0, 0x1u, Src1, 0x1u);                            \
    TestValue(e, Dest, 0u, Src0, 0x1u, Src1, 0x11111u);                        \
    TestValue(ne, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(ne, Dest, 0u, Src0, 0x1u, Src1, 0x1u);                           \
    TestValue(be, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestValue(be, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(a, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(a, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestValue(s, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestValue(s, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(ns, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(ns, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestValue(p, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(p, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestValue(np, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestValue(np, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(l, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                     \
    TestValue(l, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                     \
    TestValue(ge, Dest, 1u, Src0, 0x1u, Src1, 0x80000000u);                    \
    TestValue(ge, Dest, 0u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(le, Dest, 1u, Src0, 0x80000000u, Src1, 0x1u);                    \
    TestValue(le, Dest, 0u, Src0, 0x1u, Src1, 0x80000000u);                    \
  } while (0)

  TestImpl(r1, r2, r3);

#undef TestImpl
#undef TestValue
#undef TestRegAddr
#undef TestRegReg
}

TEST_F(AssemblerX8664LowLevelTest, RepMovsb) {
  __ rep_movsb();

  static constexpr uint32_t ByteCount = 2;
  static constexpr uint8_t Prefix = 0xF3;
  static constexpr uint8_t Opcode = 0xA4;

  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), Prefix, Opcode);
}

TEST_F(AssemblerX8664Test, MovssXmmAddr) {
#define TestMovssXmmAddrFloatLength(FloatLength, Xmm, Value)                   \
  do {                                                                         \
    static_assert((FloatLength) == 32 || (FloatLength) == 64,                  \
                  "Invalid fp length #FloatLength");                           \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
                                                                               \
    static constexpr char TestString[] = "(" #FloatLength ", " #Xmm ")";       \
    static constexpr bool IsDouble = std::is_same<Type, double>::value;        \
    const uint32_t T0 = allocateQword();                                       \
    const Type V0 = Value;                                                     \
                                                                               \
    __ movss(IceType_f##FloatLength, Encoded_Xmm_##Xmm(), dwordAddress(T0));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    if (IsDouble) {                                                            \
      test.setQwordTo(T0, static_cast<double>(V0));                            \
    } else {                                                                   \
      test.setDwordTo(T0, static_cast<float>(V0));                             \
    }                                                                          \
    test.run();                                                                \
    ASSERT_DOUBLE_EQ(Value, test.Xmm<Type>())                                  \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovssXmmAddr(FloatLength)                                          \
  do {                                                                         \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
    for (const Type Value : {0.0, -0.0, 1.0, -1.0, 3.14, 99999.9999}) {        \
      TestMovssXmmAddrFloatLength(FloatLength, xmm0, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm1, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm2, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm3, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm4, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm5, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm6, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm7, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm8, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm9, Value);                   \
      TestMovssXmmAddrFloatLength(FloatLength, xmm10, Value);                  \
      TestMovssXmmAddrFloatLength(FloatLength, xmm11, Value);                  \
      TestMovssXmmAddrFloatLength(FloatLength, xmm12, Value);                  \
      TestMovssXmmAddrFloatLength(FloatLength, xmm13, Value);                  \
      TestMovssXmmAddrFloatLength(FloatLength, xmm14, Value);                  \
      TestMovssXmmAddrFloatLength(FloatLength, xmm15, Value);                  \
    }                                                                          \
  } while (0)

  TestMovssXmmAddr(32);
  TestMovssXmmAddr(64);

#undef TestMovssXmmAddr
#undef TestMovssXmmAddrType
}

TEST_F(AssemblerX8664Test, MovssAddrXmm) {
#define TestMovssAddrXmmFloatLength(FloatLength, Xmm, Value)                   \
  do {                                                                         \
    static_assert((FloatLength) == 32 || (FloatLength) == 64,                  \
                  "Invalid fp length #FloatLength");                           \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
                                                                               \
    static constexpr char TestString[] = "(" #FloatLength ", " #Xmm ")";       \
    static constexpr bool IsDouble = std::is_same<Type, double>::value;        \
    const uint32_t T0 = allocateQword();                                       \
    const Type V0 = Value;                                                     \
    const uint32_t T1 = allocateQword();                                       \
    static_assert(std::numeric_limits<Type>::has_quiet_NaN,                    \
                  "f" #FloatLength " does not have quiet nan.");               \
    const Type V1 = std::numeric_limits<Type>::quiet_NaN();                    \
                                                                               \
    __ movss(IceType_f##FloatLength, Encoded_Xmm_##Xmm(), dwordAddress(T0));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    if (IsDouble) {                                                            \
      test.setQwordTo(T0, static_cast<double>(V0));                            \
      test.setQwordTo(T1, static_cast<double>(V1));                            \
    } else {                                                                   \
      test.setDwordTo(T0, static_cast<float>(V0));                             \
      test.setDwordTo(T1, static_cast<float>(V1));                             \
    }                                                                          \
    test.run();                                                                \
    ASSERT_DOUBLE_EQ(Value, test.Xmm<Type>())                                  \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovssAddrXmm(FloatLength)                                          \
  do {                                                                         \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
    for (const Type Value : {0.0, -0.0, 1.0, -1.0, 3.14, 99999.9999}) {        \
      TestMovssAddrXmmFloatLength(FloatLength, xmm0, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm1, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm2, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm3, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm4, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm5, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm6, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm7, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm8, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm9, Value);                   \
      TestMovssAddrXmmFloatLength(FloatLength, xmm10, Value);                  \
      TestMovssAddrXmmFloatLength(FloatLength, xmm11, Value);                  \
      TestMovssAddrXmmFloatLength(FloatLength, xmm12, Value);                  \
      TestMovssAddrXmmFloatLength(FloatLength, xmm13, Value);                  \
      TestMovssAddrXmmFloatLength(FloatLength, xmm14, Value);                  \
      TestMovssAddrXmmFloatLength(FloatLength, xmm15, Value);                  \
    }                                                                          \
  } while (0)

  TestMovssAddrXmm(32);
  TestMovssAddrXmm(64);

#undef TestMovssAddrXmm
#undef TestMovssAddrXmmType
}

TEST_F(AssemblerX8664Test, MovssXmmXmm) {
#define TestMovssXmmXmmFloatLength(FloatLength, Src, Dst, Value)               \
  do {                                                                         \
    static_assert((FloatLength) == 32 || (FloatLength) == 64,                  \
                  "Invalid fp length #FloatLength");                           \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
                                                                               \
    static constexpr char TestString[] =                                       \
        "(" #FloatLength ", " #Src ", " #Dst ")";                              \
    static constexpr bool IsDouble = std::is_same<Type, double>::value;        \
    const uint32_t T0 = allocateQword();                                       \
    const Type V0 = Value;                                                     \
    const uint32_t T1 = allocateQword();                                       \
    static_assert(std::numeric_limits<Type>::has_quiet_NaN,                    \
                  "f" #FloatLength " does not have quiet nan.");               \
    const Type V1 = std::numeric_limits<Type>::quiet_NaN();                    \
                                                                               \
    __ movss(IceType_f##FloatLength, Encoded_Xmm_##Src(), dwordAddress(T0));   \
    __ movss(IceType_f##FloatLength, Encoded_Xmm_##Dst(), dwordAddress(T1));   \
    __ movss(IceType_f##FloatLength, Encoded_Xmm_##Dst(),                      \
             Encoded_Xmm_##Src());                                             \
                                                                               \
    AssembledTest test = assemble();                                           \
    if (IsDouble) {                                                            \
      test.setQwordTo(T0, static_cast<double>(V0));                            \
      test.setQwordTo(T1, static_cast<double>(V1));                            \
    } else {                                                                   \
      test.setDwordTo(T0, static_cast<float>(V0));                             \
      test.setDwordTo(T1, static_cast<float>(V1));                             \
    }                                                                          \
    test.run();                                                                \
    ASSERT_DOUBLE_EQ(Value, test.Dst<Type>())                                  \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovssXmmXmm(FloatLength)                                           \
  do {                                                                         \
    using Type = std::conditional<FloatLength == 32, float, double>::type;     \
    for (const Type Value : {0.0, -0.0, 1.0, -1.0, 3.14, 99999.9999}) {        \
      TestMovssXmmXmmFloatLength(FloatLength, xmm0, xmm1, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm1, xmm2, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm2, xmm3, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm3, xmm4, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm4, xmm5, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm5, xmm6, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm6, xmm7, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm7, xmm8, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm8, xmm9, Value);              \
      TestMovssXmmXmmFloatLength(FloatLength, xmm9, xmm10, Value);             \
      TestMovssXmmXmmFloatLength(FloatLength, xmm10, xmm11, Value);            \
      TestMovssXmmXmmFloatLength(FloatLength, xmm11, xmm12, Value);            \
      TestMovssXmmXmmFloatLength(FloatLength, xmm12, xmm13, Value);            \
      TestMovssXmmXmmFloatLength(FloatLength, xmm13, xmm14, Value);            \
      TestMovssXmmXmmFloatLength(FloatLength, xmm14, xmm15, Value);            \
      TestMovssXmmXmmFloatLength(FloatLength, xmm15, xmm0, Value);             \
    }                                                                          \
  } while (0)

  TestMovssXmmXmm(32);
  TestMovssXmmXmm(64);

#undef TestMovssXmmXmm
#undef TestMovssXmmXmmType
}

TEST_F(AssemblerX8664Test, MovdToXmm) {
#define TestMovdXmmReg32(Src, Dst, Value)                                      \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = 0xFFFFFFFF00000000ull;                                 \
                                                                               \
    __ mov(IceType_i32, Encoded_GPR_##Src(), Immediate(Value));                \
    __ movss(IceType_f64, Encoded_Xmm_##Dst(), dwordAddress(T0));              \
    __ movd(IceType_i32, Encoded_Xmm_##Dst(), Encoded_GPR_##Src());            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdXmmReg64(Src, Dst, Value)                                      \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = 0xFFFFFFFF00000000ull;                                 \
    const uint64_t Expected = (static_cast<uint64_t>(Value) << 32) | (Value);  \
                                                                               \
    __ movabs(Encoded_GPR_##Src(), Expected);                                  \
    __ movss(IceType_f64, Encoded_Xmm_##Dst(), dwordAddress(T0));              \
    __ movd(IceType_i64, Encoded_Xmm_##Dst(), Encoded_GPR_##Src());            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst<uint64_t>())                                  \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdXmmReg(Src, Dst, Value)                                        \
  do {                                                                         \
    TestMovdXmmReg32(Src, Dst, Value);                                         \
    TestMovdXmmReg64(Src, Dst, Value);                                         \
  } while (0)

#define TestMovdXmmAddr32(Dst, Value)                                          \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint32_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = 0xFFFFFFFF00000000ull;                                 \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Dst(), dwordAddress(T1));              \
    __ movd(IceType_i32, Encoded_Xmm_##Dst(), dwordAddress(T0));               \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setDwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdXmmAddr64(Dst, Value)                                          \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint32_t V0 = (static_cast<uint64_t>(Value) << 32) | (Value);        \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = 0xFFFFFFFF00000000ull;                                 \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Dst(), dwordAddress(T1));              \
    __ movd(IceType_i64, Encoded_Xmm_##Dst(), dwordAddress(T0));               \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setDwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdXmmAddr(Dst, Value)                                            \
  do {                                                                         \
    TestMovdXmmAddr32(Dst, Value);                                             \
    TestMovdXmmAddr64(Dst, Value);                                             \
  } while (0)

#define TestMovd(Dst)                                                          \
  do {                                                                         \
    for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {   \
      TestMovdXmmReg(r1, Dst, Value);                                          \
      TestMovdXmmReg(r2, Dst, Value);                                          \
      TestMovdXmmReg(r3, Dst, Value);                                          \
      TestMovdXmmReg(r4, Dst, Value);                                          \
      TestMovdXmmReg(r5, Dst, Value);                                          \
      TestMovdXmmReg(r6, Dst, Value);                                          \
      TestMovdXmmReg(r7, Dst, Value);                                          \
      TestMovdXmmReg(r8, Dst, Value);                                          \
      TestMovdXmmReg(r10, Dst, Value);                                         \
      TestMovdXmmReg(r11, Dst, Value);                                         \
      TestMovdXmmReg(r12, Dst, Value);                                         \
      TestMovdXmmReg(r13, Dst, Value);                                         \
      TestMovdXmmReg(r14, Dst, Value);                                         \
      TestMovdXmmReg(r15, Dst, Value);                                         \
      TestMovdXmmAddr(Dst, Value);                                             \
    }                                                                          \
  } while (0)

  TestMovd(xmm0);
  TestMovd(xmm1);
  TestMovd(xmm2);
  TestMovd(xmm3);
  TestMovd(xmm4);
  TestMovd(xmm5);
  TestMovd(xmm6);
  TestMovd(xmm7);
  TestMovd(xmm8);
  TestMovd(xmm9);
  TestMovd(xmm10);
  TestMovd(xmm11);
  TestMovd(xmm12);
  TestMovd(xmm13);
  TestMovd(xmm14);
  TestMovd(xmm15);

#undef TestMovd
#undef TestMovdXmmAddr
#undef TestMovdXmmAddr64
#undef TestMovdXmmAddr32
#undef TestMovdXmmReg
#undef TestMovdXmmReg64
#undef TestMovdXmmReg32
}

TEST_F(AssemblerX8664Test, MovdFromXmm) {
#define TestMovdRegXmm32(Src, Dst, Value)                                      \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value;                                                 \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Src(), dwordAddress(T0));              \
    __ movd(IceType_i32, Encoded_GPR_##Dst(), Encoded_Xmm_##Src());            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.contentsOfDword(T0))                                 \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdRegXmm64(Src, Dst, Value)                                      \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateDword();                                       \
    const uint64_t V0 = (static_cast<uint64_t>(Value) << 32) | (Value);        \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Src(), dwordAddress(T0));              \
    __ movd(IceType_i64, Encoded_GPR_##Dst(), Encoded_Xmm_##Src());            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.contentsOfQword(T0))                                    \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdRegXmm(Src, Dst, Value)                                        \
  do {                                                                         \
    TestMovdRegXmm32(Src, Dst, Value);                                         \
    TestMovdRegXmm64(Src, Dst, Value);                                         \
  } while (0)

#define TestMovdAddrXmm32(Src, Value)                                          \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", Addr)";                   \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value;                                                 \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = ~(Value);                                              \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Src(), dwordAddress(T0));              \
    __ movd(IceType_i32, dwordAddress(T1), Encoded_Xmm_##Src());               \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setDwordTo(T0, V0);                                                   \
    test.setDwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.contentsOfDword(T1))                                 \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdAddrXmm64(Src, Value)                                          \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = (static_cast<uint64_t>(Value) << 32) | Value;          \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~V0;                                                   \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Src(), dwordAddress(T0));              \
    __ movd(IceType_i64, dwordAddress(T1), Encoded_Xmm_##Src());               \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.contentsOfQword(T1))                                    \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

#define TestMovdAddrXmm(Src, Value)                                            \
  do {                                                                         \
    TestMovdAddrXmm32(Src, Value);                                             \
    TestMovdAddrXmm64(Src, Value);                                             \
  } while (0)

#define TestMovd(Src)                                                          \
  do {                                                                         \
    for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {   \
      TestMovdRegXmm(Src, r1, Value);                                          \
      TestMovdRegXmm(Src, r2, Value);                                          \
      TestMovdRegXmm(Src, r3, Value);                                          \
      TestMovdRegXmm(Src, r4, Value);                                          \
      TestMovdRegXmm(Src, r5, Value);                                          \
      TestMovdRegXmm(Src, r6, Value);                                          \
      TestMovdRegXmm(Src, r7, Value);                                          \
      TestMovdRegXmm(Src, r8, Value);                                          \
      TestMovdRegXmm(Src, r10, Value);                                         \
      TestMovdRegXmm(Src, r11, Value);                                         \
      TestMovdRegXmm(Src, r12, Value);                                         \
      TestMovdRegXmm(Src, r13, Value);                                         \
      TestMovdRegXmm(Src, r14, Value);                                         \
      TestMovdRegXmm(Src, r15, Value);                                         \
      TestMovdAddrXmm(Src, Value);                                             \
    }                                                                          \
  } while (0)

  TestMovd(xmm0);
  TestMovd(xmm1);
  TestMovd(xmm2);
  TestMovd(xmm3);
  TestMovd(xmm4);
  TestMovd(xmm5);
  TestMovd(xmm6);
  TestMovd(xmm7);
  TestMovd(xmm8);
  TestMovd(xmm9);
  TestMovd(xmm10);
  TestMovd(xmm11);
  TestMovd(xmm12);
  TestMovd(xmm13);
  TestMovd(xmm14);
  TestMovd(xmm15);

#undef TestMovd
#undef TestMovdAddrXmm
#undef TestMovdAddrXmm64
#undef TestMovdAddrXmm32
#undef TestMovdRegXmm
#undef TestMovdRegXmm64
#undef TestMovdRegXmm32
}

TEST_F(AssemblerX8664Test, MovqXmmAddr) {
#define TestMovd(Dst, Value)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movss(IceType_f64, Encoded_Xmm_##Dst(), dwordAddress(T1));              \
    __ movq(Encoded_Xmm_##Dst(), dwordAddress(T0));                            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

  for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {
    TestMovd(xmm0, Value);
    TestMovd(xmm1, Value);
    TestMovd(xmm2, Value);
    TestMovd(xmm3, Value);
    TestMovd(xmm4, Value);
    TestMovd(xmm5, Value);
    TestMovd(xmm6, Value);
    TestMovd(xmm7, Value);
    TestMovd(xmm8, Value);
    TestMovd(xmm9, Value);
    TestMovd(xmm10, Value);
    TestMovd(xmm11, Value);
    TestMovd(xmm12, Value);
    TestMovd(xmm13, Value);
    TestMovd(xmm14, Value);
    TestMovd(xmm15, Value);
  }

#undef TestMovd
}

TEST_F(AssemblerX8664Test, MovqAddrXmm) {
#define TestMovd(Dst, Value)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movq(Encoded_Xmm_##Dst(), dwordAddress(T0));                            \
    __ movq(dwordAddress(T1), Encoded_Xmm_##Dst());                            \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

  for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {
    TestMovd(xmm0, Value);
    TestMovd(xmm1, Value);
    TestMovd(xmm2, Value);
    TestMovd(xmm3, Value);
    TestMovd(xmm4, Value);
    TestMovd(xmm5, Value);
    TestMovd(xmm6, Value);
    TestMovd(xmm7, Value);
    TestMovd(xmm8, Value);
    TestMovd(xmm9, Value);
    TestMovd(xmm10, Value);
    TestMovd(xmm11, Value);
    TestMovd(xmm12, Value);
    TestMovd(xmm13, Value);
    TestMovd(xmm14, Value);
    TestMovd(xmm15, Value);
  }

#undef TestMovd
}

TEST_F(AssemblerX8664Test, MovqXmmXmm) {
#define TestMovd(Src, Dst, Value)                                              \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movq(Encoded_Xmm_##Src(), dwordAddress(T0));                            \
    __ movq(Encoded_Xmm_##Dst(), dwordAddress(T1));                            \
    __ movq(Encoded_Xmm_##Dst(), Encoded_Xmm_##Src());                         \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.setQwordTo(T0, V0);                                                   \
    test.setQwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Value, test.Dst<uint64_t>())                                     \
        << TestString << " value is " << Value;                                \
    reset();                                                                   \
  } while (0)

  for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {
    TestMovd(xmm0, xmm1, Value);
    TestMovd(xmm1, xmm2, Value);
    TestMovd(xmm2, xmm3, Value);
    TestMovd(xmm3, xmm4, Value);
    TestMovd(xmm4, xmm5, Value);
    TestMovd(xmm5, xmm6, Value);
    TestMovd(xmm6, xmm7, Value);
    TestMovd(xmm7, xmm8, Value);
    TestMovd(xmm8, xmm9, Value);
    TestMovd(xmm9, xmm10, Value);
    TestMovd(xmm10, xmm11, Value);
    TestMovd(xmm11, xmm12, Value);
    TestMovd(xmm12, xmm13, Value);
    TestMovd(xmm13, xmm14, Value);
    TestMovd(xmm14, xmm15, Value);
    TestMovd(xmm15, xmm0, Value);
  }

#undef TestMovd
}

TEST_F(AssemblerX8664Test, MovupsXmmAddr) {
#define TestMovups(Dst)                                                        \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ")";                         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
                                                                               \
    __ movups(Encoded_Xmm_##Dst(), dwordAddress(T0));                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Dst<Dqword>()) << TestString;                           \
    reset();                                                                   \
  } while (0)

  TestMovups(xmm0);
  TestMovups(xmm1);
  TestMovups(xmm2);
  TestMovups(xmm3);
  TestMovups(xmm4);
  TestMovups(xmm5);
  TestMovups(xmm6);
  TestMovups(xmm7);
  TestMovups(xmm8);
  TestMovups(xmm9);
  TestMovups(xmm10);
  TestMovups(xmm11);
  TestMovups(xmm12);
  TestMovups(xmm13);
  TestMovups(xmm14);
  TestMovups(xmm15);

#undef TestMovups
}

TEST_F(AssemblerX8664Test, MovupsAddrXmm) {
#define TestMovups(Src)                                                        \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ")";                         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T0));                          \
    __ movups(dwordAddress(T1), Encoded_Xmm_##Src());                          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.contentsOfDqword(T1)) << TestString;                    \
    reset();                                                                   \
  } while (0)

  TestMovups(xmm0);
  TestMovups(xmm1);
  TestMovups(xmm2);
  TestMovups(xmm3);
  TestMovups(xmm4);
  TestMovups(xmm5);
  TestMovups(xmm6);
  TestMovups(xmm7);
  TestMovups(xmm8);
  TestMovups(xmm9);
  TestMovups(xmm10);
  TestMovups(xmm11);
  TestMovups(xmm12);
  TestMovups(xmm13);
  TestMovups(xmm14);
  TestMovups(xmm15);

#undef TestMovups
}

TEST_F(AssemblerX8664Test, MovupsXmmXmm) {
#define TestMovups(Dst, Src)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ")";               \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T0));                          \
    __ movups(Encoded_Xmm_##Dst(), dwordAddress(T1));                          \
    __ movups(Encoded_Xmm_##Dst(), Encoded_Xmm_##Src());                       \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Dst<Dqword>()) << TestString;                           \
    reset();                                                                   \
  } while (0)

  TestMovups(xmm0, xmm1);
  TestMovups(xmm1, xmm2);
  TestMovups(xmm2, xmm3);
  TestMovups(xmm3, xmm4);
  TestMovups(xmm4, xmm5);
  TestMovups(xmm5, xmm6);
  TestMovups(xmm6, xmm7);
  TestMovups(xmm7, xmm8);
  TestMovups(xmm8, xmm9);
  TestMovups(xmm9, xmm10);
  TestMovups(xmm10, xmm11);
  TestMovups(xmm11, xmm12);
  TestMovups(xmm12, xmm13);
  TestMovups(xmm13, xmm14);
  TestMovups(xmm14, xmm15);
  TestMovups(xmm15, xmm0);

#undef TestMovups
}

TEST_F(AssemblerX8664Test, MovapsXmmXmm) {
#define TestMovaps(Dst, Src)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ")";               \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T0));                          \
    __ movups(Encoded_Xmm_##Dst(), dwordAddress(T1));                          \
    __ movaps(Encoded_Xmm_##Dst(), Encoded_Xmm_##Src());                       \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Dst<Dqword>()) << TestString;                           \
    reset();                                                                   \
  } while (0)

  TestMovaps(xmm0, xmm1);
  TestMovaps(xmm1, xmm2);
  TestMovaps(xmm2, xmm3);
  TestMovaps(xmm3, xmm4);
  TestMovaps(xmm4, xmm5);
  TestMovaps(xmm5, xmm6);
  TestMovaps(xmm6, xmm7);
  TestMovaps(xmm7, xmm8);
  TestMovaps(xmm8, xmm9);
  TestMovaps(xmm9, xmm10);
  TestMovaps(xmm10, xmm11);
  TestMovaps(xmm11, xmm12);
  TestMovaps(xmm12, xmm13);
  TestMovaps(xmm13, xmm14);
  TestMovaps(xmm14, xmm15);
  TestMovaps(xmm15, xmm0);

#undef TestMovaps
}

TEST_F(AssemblerX8664Test, Movhlps_Movlhps) {
#define TestImplSingle(Dst, Src, Inst, Expect)                                 \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Inst ")";    \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(uint64_t(0xAAAAAAAABBBBBBBBull),                           \
                    uint64_t(0xCCCCCCCCDDDDDDDDull));                          \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(uint64_t(0xEEEEEEEEFFFFFFFFull),                           \
                    uint64_t(0x9999999988888888ull));                          \
                                                                               \
    __ movups(Encoded_Xmm_##Dst(), dwordAddress(T0));                          \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T1));                          \
    __ Inst(Encoded_Xmm_##Dst(), Encoded_Xmm_##Src());                         \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Dqword Expect, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSingle(                                                            \
        Dst, Src, movhlps,                                                     \
        (uint64_t(0x9999999988888888ull), uint64_t(0xCCCCCCCCDDDDDDDDull)));   \
    TestImplSingle(                                                            \
        Dst, Src, movlhps,                                                     \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xEEEEEEEEFFFFFFFFull)));   \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm8);
  TestImpl(xmm8, xmm9);
  TestImpl(xmm9, xmm10);
  TestImpl(xmm10, xmm11);
  TestImpl(xmm11, xmm12);
  TestImpl(xmm12, xmm13);
  TestImpl(xmm13, xmm14);
  TestImpl(xmm14, xmm15);
  TestImpl(xmm15, xmm0);

#undef TestImpl
#undef TestImplSingle
}

TEST_F(AssemblerX8664Test, Movmsk) {
#define TestMovmskGPRXmm(GPR, Src, Value1, Expected, Inst)                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #GPR ", " #Src ", " #Value1 ", " #Expected ", " #Inst ")";         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value1;                                                    \
                                                                               \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T0));                          \
    __ Inst(IceType_v4f32, Encoded_GPR_##GPR(), Encoded_Xmm_##Src());          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.GPR()) << TestString;                             \
    reset();                                                                   \
  } while (0)

#define TestMovmsk(GPR, Src)                                                   \
  do {                                                                         \
    TestMovmskGPRXmm(GPR, Src, (-1.0, 1.0, -1.0, 1.0), 0x05ul, movmsk);        \
  } while (0)

  TestMovmsk(r1, xmm0);
  TestMovmsk(r2, xmm1);
  TestMovmsk(r3, xmm2);
  TestMovmsk(r4, xmm3);
  TestMovmsk(r5, xmm4);
  TestMovmsk(r6, xmm5);
  TestMovmsk(r7, xmm6);
  TestMovmsk(r8, xmm7);
  TestMovmsk(r10, xmm8);
  TestMovmsk(r11, xmm9);
  TestMovmsk(r12, xmm10);
  TestMovmsk(r13, xmm11);
  TestMovmsk(r14, xmm12);
  TestMovmsk(r15, xmm13);
  TestMovmsk(r1, xmm14);
  TestMovmsk(r2, xmm15);

#undef TestMovmskGPRXmm
#undef TestMovmsk
}

TEST_F(AssemblerX8664Test, Pmovsxdq) {
#define TestPmovsxdqXmmXmm(Dst, Src, Value1)                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Value1 ")";  \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value1;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(uint64_t(0), uint64_t(0));                                 \
                                                                               \
    __ movups(Encoded_Xmm_##Src(), dwordAddress(T0));                          \
    __ movups(Encoded_Xmm_##Dst(), dwordAddress(T1));                          \
    __ pmovsxdq(Encoded_Xmm_##Dst(), Encoded_Xmm_##Src());                     \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    const Dqword Expected(uint64_t(V0.I32[0]), uint64_t(V0.I32[1]));           \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestPmovsxdq(Dst, Src)                                                 \
  do {                                                                         \
    TestPmovsxdqXmmXmm(                                                        \
        Dst, Src,                                                              \
        (uint64_t(0x700000007FFFFFFFull), uint64_t(0xAAAAAAAAEEEEEEEEull)));   \
    TestPmovsxdqXmmXmm(                                                        \
        Dst, Src,                                                              \
        (uint64_t(0x800000007FFFFFFFull), uint64_t(0xAAAAAAAAEEEEEEEEull)));   \
    TestPmovsxdqXmmXmm(                                                        \
        Dst, Src,                                                              \
        (uint64_t(0x70000000FFFFFFFFull), uint64_t(0xAAAAAAAAEEEEEEEEull)));   \
    TestPmovsxdqXmmXmm(                                                        \
        Dst, Src,                                                              \
        (uint64_t(0x80000000FFFFFFFFull), uint64_t(0xAAAAAAAAEEEEEEEEull)));   \
  } while (0)

  TestPmovsxdq(xmm0, xmm1);
  TestPmovsxdq(xmm1, xmm2);
  TestPmovsxdq(xmm2, xmm3);
  TestPmovsxdq(xmm3, xmm4);
  TestPmovsxdq(xmm4, xmm5);
  TestPmovsxdq(xmm5, xmm6);
  TestPmovsxdq(xmm6, xmm7);
  TestPmovsxdq(xmm7, xmm8);
  TestPmovsxdq(xmm8, xmm9);
  TestPmovsxdq(xmm9, xmm10);
  TestPmovsxdq(xmm10, xmm11);
  TestPmovsxdq(xmm11, xmm12);
  TestPmovsxdq(xmm12, xmm13);
  TestPmovsxdq(xmm13, xmm14);
  TestPmovsxdq(xmm14, xmm15);
  TestPmovsxdq(xmm15, xmm0);

#undef TestPmovsxdq
#undef TestPmovsxdqXmmXmm
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
