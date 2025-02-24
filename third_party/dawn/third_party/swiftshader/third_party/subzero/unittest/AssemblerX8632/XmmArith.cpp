//===- subzero/unittest/AssemblerX8632/XmmArith.cpp -----------------------===//
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

TEST_F(AssemblerX8632Test, ArithSS) {
#define TestArithSSXmmXmm(FloatSize, Src, Value0, Dst, Value1, Inst, Op)       \
  do {                                                                         \
    static_assert(FloatSize == 32 || FloatSize == 64,                          \
                  "Invalid fp size " #FloatSize);                              \
    static constexpr char TestString[] =                                       \
        "(" #FloatSize ", " #Src ", " #Value0 ", " #Dst ", " #Value1           \
        ", " #Inst ", " #Op ")";                                               \
    static constexpr bool IsDouble = FloatSize == 64;                          \
    using Type = std::conditional<IsDouble, double, float>::type;              \
    const uint32_t T0 = allocateQword();                                       \
    const Type V0 = Value0;                                                    \
    const uint32_t T1 = allocateQword();                                       \
    const Type V1 = Value1;                                                    \
                                                                               \
    __ movss(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             dwordAddress(T0));                                                \
    __ movss(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Src,             \
             dwordAddress(T1));                                                \
    __ Inst(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,              \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    if (IsDouble) {                                                            \
      test.setQwordTo(T0, static_cast<double>(V0));                            \
      test.setQwordTo(T1, static_cast<double>(V1));                            \
    } else {                                                                   \
      test.setDwordTo(T0, static_cast<float>(V0));                             \
      test.setDwordTo(T1, static_cast<float>(V1));                             \
    }                                                                          \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_DOUBLE_EQ(V0 Op V1, test.Dst<Type>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestArithSSXmmAddr(FloatSize, Value0, Dst, Value1, Inst, Op)           \
  do {                                                                         \
    static_assert(FloatSize == 32 || FloatSize == 64,                          \
                  "Invalid fp size " #FloatSize);                              \
    static constexpr char TestString[] =                                       \
        "(" #FloatSize ", Addr, " #Value0 ", " #Dst ", " #Value1 ", " #Inst    \
        ", " #Op ")";                                                          \
    static constexpr bool IsDouble = FloatSize == 64;                          \
    using Type = std::conditional<IsDouble, double, float>::type;              \
    const uint32_t T0 = allocateQword();                                       \
    const Type V0 = Value0;                                                    \
    const uint32_t T1 = allocateQword();                                       \
    const Type V1 = Value1;                                                    \
                                                                               \
    __ movss(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             dwordAddress(T0));                                                \
    __ Inst(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,              \
            dwordAddress(T1));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    if (IsDouble) {                                                            \
      test.setQwordTo(T0, static_cast<double>(V0));                            \
      test.setQwordTo(T1, static_cast<double>(V1));                            \
    } else {                                                                   \
      test.setDwordTo(T0, static_cast<float>(V0));                             \
      test.setDwordTo(T1, static_cast<float>(V1));                             \
    }                                                                          \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_DOUBLE_EQ(V0 Op V1, test.Dst<Type>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestArithSS(FloatSize, Src, Dst0, Dst1)                                \
  do {                                                                         \
    TestArithSSXmmXmm(FloatSize, Src, 1.0, Dst0, 10.0, addss, +);              \
    TestArithSSXmmAddr(FloatSize, 2.0, Dst1, 20.0, addss, +);                  \
    TestArithSSXmmXmm(FloatSize, Src, 3.0, Dst0, 30.0, subss, -);              \
    TestArithSSXmmAddr(FloatSize, 4.0, Dst1, 40.0, subss, -);                  \
    TestArithSSXmmXmm(FloatSize, Src, 5.0, Dst0, 50.0, mulss, *);              \
    TestArithSSXmmAddr(FloatSize, 6.0, Dst1, 60.0, mulss, *);                  \
    TestArithSSXmmXmm(FloatSize, Src, 7.0, Dst0, 70.0, divss, /);              \
    TestArithSSXmmAddr(FloatSize, 8.0, Dst1, 80.0, divss, /);                  \
  } while (0)

  TestArithSS(32, xmm0, xmm1, xmm2);
  TestArithSS(32, xmm1, xmm2, xmm3);
  TestArithSS(32, xmm2, xmm3, xmm4);
  TestArithSS(32, xmm3, xmm4, xmm5);
  TestArithSS(32, xmm4, xmm5, xmm6);
  TestArithSS(32, xmm5, xmm6, xmm7);
  TestArithSS(32, xmm6, xmm7, xmm0);
  TestArithSS(32, xmm7, xmm0, xmm1);

  TestArithSS(64, xmm0, xmm1, xmm2);
  TestArithSS(64, xmm1, xmm2, xmm3);
  TestArithSS(64, xmm2, xmm3, xmm4);
  TestArithSS(64, xmm3, xmm4, xmm5);
  TestArithSS(64, xmm4, xmm5, xmm6);
  TestArithSS(64, xmm5, xmm6, xmm7);
  TestArithSS(64, xmm6, xmm7, xmm0);
  TestArithSS(64, xmm7, xmm0, xmm1);

#undef TestArithSS
#undef TestArithSSXmmAddr
#undef TestArithSSXmmXmm
}

TEST_F(AssemblerX8632Test, PArith) {
#define TestPArithXmmXmm(Dst, Value0, Src, Value1, Inst, Op, Type, Size)       \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Inst ", " #Op       \
        ", " #Type ", " #Size ")";                                             \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type##Size##_t>(V0) Op V1, test.Dst<Dqword>())          \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestPArithXmmAddr(Dst, Value0, Value1, Inst, Op, Type, Size)           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Inst ", " #Op           \
        ", " #Type ", " #Size ")";                                             \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T1));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type##Size##_t>(V0) Op V1, test.Dst<Dqword>())          \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestPArithXmmImm(Dst, Value0, Imm, Inst, Op, Type, Size)               \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Imm ", " #Inst ", " #Op ", " #Type         \
        ", " #Size ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst, Immediate(Imm));  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type##Size##_t>(V0) Op Imm, test.Dst<Dqword>())         \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestPAndnXmmXmm(Dst, Value0, Src, Value1, Type, Size)                  \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", pandn, " #Type         \
        ", " #Size ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ pandn(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                  \
             XmmRegister::Encoded_Reg_##Src);                                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(~(packedAs<Type##Size##_t>(V0)) & V1, test.Dst<Dqword>())        \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestPAndnXmmAddr(Dst, Value0, Value1, Type, Size)                      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", pandn, " #Type ", " #Size  \
        ")";                                                                   \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ pandn(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                  \
             dwordAddress(T1));                                                \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ((~packedAs<Type##Size##_t>(V0)) & V1, test.Dst<Dqword>())        \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestPArithSize(Dst, Src, Size)                                         \
  do {                                                                         \
    static_assert(Size == 8 || Size == 16 || Size == 32, "Invalid size.");     \
    if (Size != 8) {                                                           \
      TestPArithXmmXmm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          Src, (uint64_t(3u), uint64_t(0u)), psra, >>, int, Size);             \
      TestPArithXmmAddr(                                                       \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          (uint64_t(3u), uint64_t(0u)), psra, >>, int, Size);                  \
      TestPArithXmmImm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          3u, psra, >>, int, Size);                                            \
      TestPArithXmmXmm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          Src, (uint64_t(3u), uint64_t(0u)), psrl, >>, uint, Size);            \
      TestPArithXmmAddr(                                                       \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          (uint64_t(3u), uint64_t(0u)), psrl, >>, uint, Size);                 \
      TestPArithXmmImm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          3u, psrl, >>, uint, Size);                                           \
      TestPArithXmmXmm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          Src, (uint64_t(3u), uint64_t(0u)), psll, <<, uint, Size);            \
      TestPArithXmmAddr(                                                       \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          (uint64_t(3u), uint64_t(0u)), psll, <<, uint, Size);                 \
      TestPArithXmmImm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          3u, psll, <<, uint, Size);                                           \
                                                                               \
      TestPArithXmmXmm(                                                        \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          Src,                                                                 \
          (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),   \
          pmull, *, int, Size);                                                \
      TestPArithXmmAddr(                                                       \
          Dst,                                                                 \
          (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),  \
          (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),   \
          pmull, *, int, Size);                                                \
      if (Size != 16) {                                                        \
        TestPArithXmmXmm(                                                      \
            Dst,                                                               \
            (uint64_t(0x8040201008040201ull),                                  \
             uint64_t(0x8080404002020101ull)),                                 \
            Src,                                                               \
            (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)), \
            pmuludq, *, uint, Size);                                           \
        TestPArithXmmAddr(                                                     \
            Dst,                                                               \
            (uint64_t(0x8040201008040201ull),                                  \
             uint64_t(0x8080404002020101ull)),                                 \
            (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)), \
            pmuludq, *, uint, Size);                                           \
      }                                                                        \
    }                                                                          \
    TestPArithXmmXmm(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        padd, +, int, Size);                                                   \
    TestPArithXmmAddr(                                                         \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        padd, +, int, Size);                                                   \
    TestPArithXmmXmm(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        psub, -, int, Size);                                                   \
    TestPArithXmmAddr(                                                         \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        psub, -, int, Size);                                                   \
    TestPArithXmmXmm(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        pand, &, int, Size);                                                   \
    TestPArithXmmAddr(                                                         \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        pand, &, int, Size);                                                   \
                                                                               \
    TestPAndnXmmXmm(                                                           \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        int, Size);                                                            \
    TestPAndnXmmAddr(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        int, Size);                                                            \
                                                                               \
    TestPArithXmmXmm(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        por, |, int, Size);                                                    \
    TestPArithXmmAddr(                                                         \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        por, |, int, Size);                                                    \
    TestPArithXmmXmm(                                                          \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        Src,                                                                   \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        pxor, ^, int, Size);                                                   \
    TestPArithXmmAddr(                                                         \
        Dst,                                                                   \
        (uint64_t(0x8040201008040201ull), uint64_t(0x8080404002020101ull)),    \
        (uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x0123456789ABCDEull)),     \
        pxor, ^, int, Size);                                                   \
  } while (0)

#define TestPArith(Src, Dst)                                                   \
  do {                                                                         \
    TestPArithSize(Src, Dst, 8);                                               \
    TestPArithSize(Src, Dst, 16);                                              \
    TestPArithSize(Src, Dst, 32);                                              \
  } while (0)

  TestPArith(xmm0, xmm1);
  TestPArith(xmm1, xmm2);
  TestPArith(xmm2, xmm3);
  TestPArith(xmm3, xmm4);
  TestPArith(xmm4, xmm5);
  TestPArith(xmm5, xmm6);
  TestPArith(xmm6, xmm7);
  TestPArith(xmm7, xmm0);

#undef TestPArith
#undef TestPArithSize
#undef TestPAndnXmmAddr
#undef TestPAndnXmmXmm
#undef TestPArithXmmImm
#undef TestPArithXmmAddr
#undef TestPArithXmmXmm
}

TEST_F(AssemblerX8632Test, ArithPS) {
#define TestArithPSXmmXmm(FloatSize, Dst, Value0, Src, Value1, Inst, Op, Type) \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Inst ", " #Op       \
        ", " #Type ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,              \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestArithPSXmmXmmUntyped(Dst, Value0, Src, Value1, Inst, Op, Type)     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Inst ", " #Op       \
        ", " #Type ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src);   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestArithPSXmmAddrUntyped(Dst, Value0, Value1, Inst, Op, Type)         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Inst ", " #Op           \
        ", " #Type ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestMinMaxPS(FloatSize, Dst, Value0, Src, Value1, Inst, Type)          \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Inst ", " #Type     \
        ")";                                                                   \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,              \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0).Inst(V1), test.Dst<Dqword>()) << TestString;  \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestArithPSXmmAddr(FloatSize, Dst, Value0, Value1, Inst, Op, Type)     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Inst ", " #Op           \
        ", " #Type ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,              \
            dwordAddress(T1));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
                                                                               \
    reset();                                                                   \
  } while (0)

#define TestArithPS(Dst, Src)                                                  \
  do {                                                                         \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), addps, +, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), addps, +, float);             \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), subps, -, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), subps, -, float);             \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), mulps, *, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), mulps, *, float);             \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), divps, /, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), divps, /, float);             \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), andps, &, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), andps, &, float);             \
    TestArithPSXmmXmm(64, Dst, (1.0, -1000.0), Src, (0.55, 1.21), andps, &,    \
                      double);                                                 \
    TestArithPSXmmAddr(64, Dst, (1.0, -1000.0), (0.55, 1.21), andps, &,        \
                       double);                                                \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), orps, |, float);               \
    TestArithPSXmmXmm(64, Dst, (1.0, -1000.0), Src, (0.55, 1.21), orps, |,     \
                      double);                                                 \
    TestMinMaxPS(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,                    \
                 (0.55, 0.43, 0.23, 1.21), minps, float);                      \
    TestMinMaxPS(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,                    \
                 (0.55, 0.43, 0.23, 1.21), maxps, float);                      \
    TestMinMaxPS(64, Dst, (1.0, -1000.0), Src, (0.55, 1.21), minps, double);   \
    TestMinMaxPS(64, Dst, (1.0, -1000.0), Src, (0.55, 1.21), maxps, double);   \
    TestArithPSXmmXmm(32, Dst, (1.0, 100.0, -1000.0, 20.0), Src,               \
                      (0.55, 0.43, 0.23, 1.21), xorps, ^, float);              \
    TestArithPSXmmAddr(32, Dst, (1.0, 100.0, -1000.0, 20.0),                   \
                       (0.55, 0.43, 0.23, 1.21), xorps, ^, float);             \
    TestArithPSXmmXmm(64, Dst, (1.0, -1000.0), Src, (0.55, 1.21), xorps, ^,    \
                      double);                                                 \
    TestArithPSXmmAddr(64, Dst, (1.0, -1000.0), (0.55, 1.21), xorps, ^,        \
                       double);                                                \
  } while (0)

#if 0

#endif

  TestArithPS(xmm0, xmm1);
  TestArithPS(xmm1, xmm2);
  TestArithPS(xmm2, xmm3);
  TestArithPS(xmm3, xmm4);
  TestArithPS(xmm4, xmm5);
  TestArithPS(xmm5, xmm6);
  TestArithPS(xmm6, xmm7);
  TestArithPS(xmm7, xmm0);

#undef TestArithPs
#undef TestMinMaxPS
#undef TestArithPSXmmXmmUntyped
#undef TestArithPSXmmAddr
#undef TestArithPSXmmXmm
}

TEST_F(AssemblerX8632Test, Blending) {
  using f32 = float;
  using i8 = uint8_t;

#define TestBlendingXmmXmm(Dst, Value0, Src, Value1, M /*ask*/, Inst, Type)    \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #M ", " #Inst        \
        ", " #Type ")";                                                        \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
    const uint32_t Mask = allocateDqword();                                    \
    const Dqword MaskValue M;                                                  \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_xmm0, dwordAddress(Mask));              \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_##Type, XmmRegister::Encoded_Reg_##Dst,                    \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.setDqwordTo(Mask, MaskValue);                                         \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0).blendWith(V1, MaskValue), test.Dst<Dqword>()) \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestBlendingXmmAddr(Dst, Value0, Value1, M /*ask*/, Inst, Type)        \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #M ", " #Inst ", " #Type \
        ")";                                                                   \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
    const uint32_t Mask = allocateDqword();                                    \
    const Dqword MaskValue M;                                                  \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_xmm0, dwordAddress(Mask));              \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_##Type, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1)); \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.setDqwordTo(Mask, MaskValue);                                         \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0).blendWith(V1, MaskValue), test.Dst<Dqword>()) \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestBlending(Src, Dst)                                                 \
  do {                                                                         \
    TestBlendingXmmXmm(                                                        \
        Dst, (1.0, 2.0, 1.0, 2.0), Src, (-1.0, -2.0, -1.0, -2.0),              \
        (uint64_t(0x8000000000000000ull), uint64_t(0x0000000080000000ull)),    \
        blendvps, f32);                                                        \
    TestBlendingXmmAddr(                                                       \
        Dst, (1.0, 2.0, 1.0, 2.0), (-1.0, -2.0, -1.0, -2.0),                   \
        (uint64_t(0x8000000000000000ull), uint64_t(0x0000000080000000ull)),    \
        blendvps, f32);                                                        \
    TestBlendingXmmXmm(                                                        \
        Dst,                                                                   \
        (uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xBBBBBBBBBBBBBBBBull)),    \
        Src,                                                                   \
        (uint64_t(0xAAAAAAAAAAAAAAAAull), uint64_t(0xEEEEEEEEEEEEEEEEull)),    \
        (uint64_t(0x8000000000000080ull), uint64_t(0x8080808000000000ull)),    \
        pblendvb, i8);                                                         \
    TestBlendingXmmAddr(                                                       \
        Dst,                                                                   \
        (uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xBBBBBBBBBBBBBBBBull)),    \
        (uint64_t(0xAAAAAAAAAAAAAAAAull), uint64_t(0xEEEEEEEEEEEEEEEEull)),    \
        (uint64_t(0x8000000000000080ull), uint64_t(0x8080808000000000ull)),    \
        pblendvb, i8);                                                         \
  } while (0)

  /* xmm0 is taken. It is the implicit mask . */
  TestBlending(xmm1, xmm2);
  TestBlending(xmm2, xmm3);
  TestBlending(xmm3, xmm4);
  TestBlending(xmm4, xmm5);
  TestBlending(xmm5, xmm6);
  TestBlending(xmm6, xmm7);
  TestBlending(xmm7, xmm1);

#undef TestBlending
#undef TestBlendingXmmAddr
#undef TestBlendingXmmXmm
}

TEST_F(AssemblerX8632Test, Cmpps) {
#define TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, C, Op, Type)      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Src ", " #Dst ", " #C ", " #Op ")";                               \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ cmpps(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             XmmRegister::Encoded_Reg_##Src, Cond::Cmpps_##C);                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
    ;                                                                          \
    reset();                                                                   \
  } while (0)

#define TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, C, Op, Type)          \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr, " #C ", " #Op ")";  \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ cmpps(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             dwordAddress(T1), Cond::Cmpps_##C);                               \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0) Op V1, test.Dst<Dqword>()) << TestString;     \
    ;                                                                          \
    reset();                                                                   \
  } while (0)

#define TestCmppsOrdUnordXmmXmm(FloatSize, Dst, Value0, Src, Value1, C, Type)  \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ", " #Dst ", " #C ")";       \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0, 1.0, std::numeric_limits<float>::quiet_NaN(),         \
                    std::numeric_limits<float>::quiet_NaN());                  \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(1.0, std::numeric_limits<float>::quiet_NaN(), 1.0,         \
                    std::numeric_limits<float>::quiet_NaN());                  \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ cmpps(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             XmmRegister::Encoded_Reg_##Src, Cond::Cmpps_##C);                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0).C(V1), test.Dst<Dqword>()) << TestString;     \
    ;                                                                          \
    reset();                                                                   \
  } while (0)

#define TestCmppsOrdUnordXmmAddr(FloatSize, Dst, Value0, Value1, C, Type)      \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #C ")";                 \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0, 1.0, std::numeric_limits<float>::quiet_NaN(),         \
                    std::numeric_limits<float>::quiet_NaN());                  \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(1.0, std::numeric_limits<float>::quiet_NaN(), 1.0,         \
                    std::numeric_limits<float>::quiet_NaN());                  \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ cmpps(IceType_f##FloatSize, XmmRegister::Encoded_Reg_##Dst,             \
             dwordAddress(T1), Cond::Cmpps_##C);                               \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(packedAs<Type>(V0).C(V1), test.Dst<Dqword>()) << TestString;     \
    ;                                                                          \
    reset();                                                                   \
  } while (0)

#define TestCmpps(FloatSize, Dst, Value0, Src, Value1, Type)                   \
  do {                                                                         \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    TestCmppsOrdUnordXmmXmm(FloatSize, Dst, Value0, Src, Value1, unord, Type); \
    TestCmppsOrdUnordXmmAddr(FloatSize, Dst, Value0, Value1, unord, Type);     \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    TestCmppsXmmXmm(FloatSize, Dst, Value0, Src, Value1, eq, ==, Type);        \
    TestCmppsXmmAddr(FloatSize, Dst, Value0, Value1, eq, ==, Type);            \
    if (FloatSize == 32) {                                                     \
      TestCmppsOrdUnordXmmXmm(32, Dst,                                         \
                              (1.0, 1.0,                                       \
                               std::numeric_limits<float>::quiet_NaN(),        \
                               std::numeric_limits<float>::quiet_NaN()),       \
                              Src,                                             \
                              (1.0, std::numeric_limits<float>::quiet_NaN(),   \
                               1.0, std::numeric_limits<float>::quiet_NaN()),  \
                              unord, Type);                                    \
      TestCmppsOrdUnordXmmAddr(32, Dst,                                        \
                               (1.0, 1.0,                                      \
                                std::numeric_limits<float>::quiet_NaN(),       \
                                std::numeric_limits<float>::quiet_NaN()),      \
                               (1.0, std::numeric_limits<float>::quiet_NaN(),  \
                                1.0, std::numeric_limits<float>::quiet_NaN()), \
                               unord, Type);                                   \
    } else {                                                                   \
      TestCmppsOrdUnordXmmXmm(                                                 \
          64, Dst, (1.0, std::numeric_limits<double>::quiet_NaN()), Src,       \
          (std::numeric_limits<double>::quiet_NaN(),                           \
           std::numeric_limits<double>::quiet_NaN()),                          \
          unord, Type);                                                        \
      TestCmppsOrdUnordXmmXmm(64, Dst, (1.0, 1.0), Src,                        \
                              (1.0, std::numeric_limits<double>::quiet_NaN()), \
                              unord, Type);                                    \
      TestCmppsOrdUnordXmmAddr(                                                \
          64, Dst, (1.0, std::numeric_limits<double>::quiet_NaN()),            \
          (std::numeric_limits<double>::quiet_NaN(),                           \
           std::numeric_limits<double>::quiet_NaN()),                          \
          unord, Type);                                                        \
      TestCmppsOrdUnordXmmAddr(                                                \
          64, Dst, (1.0, 1.0),                                                 \
          (1.0, std::numeric_limits<double>::quiet_NaN()), unord, Type);       \
    }                                                                          \
  } while (0)

#define TestCmppsSize(FloatSize, Value0, Value1, Type)                         \
  do {                                                                         \
    TestCmpps(FloatSize, xmm0, Value0, xmm1, Value1, Type);                    \
    TestCmpps(FloatSize, xmm1, Value0, xmm2, Value1, Type);                    \
    TestCmpps(FloatSize, xmm2, Value0, xmm3, Value1, Type);                    \
    TestCmpps(FloatSize, xmm3, Value0, xmm4, Value1, Type);                    \
    TestCmpps(FloatSize, xmm4, Value0, xmm5, Value1, Type);                    \
    TestCmpps(FloatSize, xmm5, Value0, xmm6, Value1, Type);                    \
    TestCmpps(FloatSize, xmm6, Value0, xmm7, Value1, Type);                    \
    TestCmpps(FloatSize, xmm7, Value0, xmm0, Value1, Type);                    \
  } while (0)

  TestCmppsSize(32, (-1.0, 1.0, 3.14, 1024.5), (-1.0, 1.0, 3.14, 1024.5),
                float);
  TestCmppsSize(64, (1.0, -1000.0), (0.55, 1.21), double);

#undef TestCmpps
#undef TestCmppsOrdUnordXmmAddr
#undef TestCmppsOrdUnordXmmXmm
#undef TestCmppsXmmAddr
#undef TestCmppsXmmXmm
}

TEST_F(AssemblerX8632Test, Sqrtps_Rsqrtps_Reciprocalps_Sqrtpd) {
#define TestImplSingle(Dst, Inst, Expect)                                      \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Inst ")";              \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0, 4.0, 20.0, 3.14);                                     \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(XmmRegister::Encoded_Reg_##Dst);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
    ASSERT_EQ(Dqword Expect, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst)                                                          \
  do {                                                                         \
    TestImplSingle(                                                            \
        Dst, sqrtps,                                                           \
        (uint64_t(0x400000003F800000ull), uint64_t(0x3FE2D10B408F1BBDull)));   \
    TestImplSingle(                                                            \
        Dst, rsqrtps,                                                          \
        (uint64_t(0x3EFFF0003F7FF000ull), uint64_t(0x3F1078003E64F000ull)));   \
    TestImplSingle(                                                            \
        Dst, reciprocalps,                                                     \
        (uint64_t(0x3E7FF0003F7FF000ull), uint64_t(0x3EA310003D4CC000ull)));   \
                                                                               \
    TestImplSingle(                                                            \
        Dst, sqrtpd,                                                           \
        (uint64_t(0x4036A09E9365F5F3ull), uint64_t(0x401C42FAE40282A8ull)));   \
  } while (0)

  TestImpl(xmm0);
  TestImpl(xmm1);
  TestImpl(xmm2);
  TestImpl(xmm3);
  TestImpl(xmm4);
  TestImpl(xmm5);
  TestImpl(xmm6);
  TestImpl(xmm7);

#undef TestImpl
#undef TestImplSingle
}

TEST_F(AssemblerX8632Test, Unpck) {
  const Dqword V0(uint64_t(0xAAAAAAAABBBBBBBBull),
                  uint64_t(0xCCCCCCCCDDDDDDDDull));
  const Dqword V1(uint64_t(0xEEEEEEEEFFFFFFFFull),
                  uint64_t(0x9999999988888888ull));

  const Dqword unpcklpsExpected(uint64_t(0xFFFFFFFFBBBBBBBBull),
                                uint64_t(0xEEEEEEEEAAAAAAAAull));
  const Dqword unpcklpdExpected(uint64_t(0xAAAAAAAABBBBBBBBull),
                                uint64_t(0xEEEEEEEEFFFFFFFFull));
  const Dqword unpckhpsExpected(uint64_t(0x88888888DDDDDDDDull),
                                uint64_t(0x99999999CCCCCCCCull));
  const Dqword unpckhpdExpected(uint64_t(0xCCCCCCCCDDDDDDDDull),
                                uint64_t(0x9999999988888888ull));

#define TestImplSingle(Dst, Src, Inst)                                         \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Inst ")";    \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src);   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Expected, test.Dst<Dqword>()) << TestString;               \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSingle(Dst, Src, unpcklps);                                        \
    TestImplSingle(Dst, Src, unpcklpd);                                        \
    TestImplSingle(Dst, Src, unpckhps);                                        \
    TestImplSingle(Dst, Src, unpckhpd);                                        \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplSingle
}

TEST_F(AssemblerX8632Test, Shufp) {
  const Dqword V0(uint64_t(0x1111111122222222ull),
                  uint64_t(0x5555555577777777ull));
  const Dqword V1(uint64_t(0xAAAAAAAABBBBBBBBull),
                  uint64_t(0xCCCCCCCCDDDDDDDDull));

  const uint8_t pshufdImm = 0x63;
  const Dqword pshufdExpected(uint64_t(0xBBBBBBBBCCCCCCCCull),
                              uint64_t(0xAAAAAAAADDDDDDDDull));

  const uint8_t shufpsImm = 0xf9;
  const Dqword shufpsExpected(uint64_t(0x7777777711111111ull),
                              uint64_t(0xCCCCCCCCCCCCCCCCull));

#define TestImplSingleXmmXmm(Dst, Src, Inst)                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Inst ")";    \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_f32, XmmRegister::Encoded_Reg_##Dst,                       \
            XmmRegister::Encoded_Reg_##Src, Immediate(Inst##Imm));             \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Expected, test.Dst<Dqword>()) << TestString;               \
    reset();                                                                   \
  } while (0)

#define TestImplSingleXmmAddr(Dst, Inst)                                       \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr, " #Inst ")";        \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_f32, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1),     \
            Immediate(Inst##Imm));                                             \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Expected, test.Dst<Dqword>()) << TestString;               \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSingleXmmXmm(Dst, Src, pshufd);                                    \
    TestImplSingleXmmAddr(Dst, pshufd);                                        \
    TestImplSingleXmmXmm(Dst, Src, shufps);                                    \
    TestImplSingleXmmAddr(Dst, shufps);                                        \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplSingleXmmAddr
#undef TestImplSingleXmmXmm
}

TEST_F(AssemblerX8632Test, Punpckl) {
  const Dqword V0_v4i32(uint64_t(0x1111111122222222ull),
                        uint64_t(0x5555555577777777ull));
  const Dqword V1_v4i32(uint64_t(0xAAAAAAAABBBBBBBBull),
                        uint64_t(0xCCCCCCCCDDDDDDDDull));
  const Dqword Expected_v4i32(uint64_t(0xBBBBBBBB22222222ull),
                              uint64_t(0xAAAAAAAA11111111ull));

  const Dqword V0_v8i16(uint64_t(0x1111222233334444ull),
                        uint64_t(0x5555666677778888ull));
  const Dqword V1_v8i16(uint64_t(0xAAAABBBBCCCCDDDDull),
                        uint64_t(0xEEEEFFFF00009999ull));
  const Dqword Expected_v8i16(uint64_t(0xCCCC3333DDDD4444ull),
                              uint64_t(0xAAAA1111BBBB2222ull));

  const Dqword V0_v16i8(uint64_t(0x1122334455667788ull),
                        uint64_t(0x99AABBCCDDEEFF00ull));
  const Dqword V1_v16i8(uint64_t(0xFFEEDDCCBBAA9900ull),
                        uint64_t(0xBAADF00DFEEDFACEull));
  const Dqword Expected_v16i8(uint64_t(0xBB55AA6699770088ull),
                              uint64_t(0xFF11EE22DD33CC44ull));

#define TestImplXmmXmm(Dst, Src, Inst, Ty)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Inst ", " #Ty ")";                            \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst,                      \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImplXmmAddr(Dst, Inst, Ty)                                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, " #Inst ", " #Ty ")";                                \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplXmmXmm(Dst, Src, punpckl, v4i32);                                  \
    TestImplXmmAddr(Dst, punpckl, v4i32);                                      \
    TestImplXmmXmm(Dst, Src, punpckl, v8i16);                                  \
    TestImplXmmAddr(Dst, punpckl, v8i16);                                      \
    TestImplXmmXmm(Dst, Src, punpckl, v16i8);                                  \
    TestImplXmmAddr(Dst, punpckl, v16i8);                                      \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplXmmAddr
#undef TestImplXmmXmm
}

TEST_F(AssemblerX8632Test, Packss) {
  const Dqword V0_v4i32(uint64_t(0x0001000000001234ull),
                        uint64_t(0x7FFFFFFF80000000ull));
  const Dqword V1_v4i32(uint64_t(0xFFFFFFFEFFFFFFFFull),
                        uint64_t(0x0000800100007FFEull));
  const Dqword Expected_v4i32(uint64_t(0x7FFF80007FFF1234ull),
                              uint64_t(0x7FFF7FFEFFFEFFFFull));

  const Dqword V0_v8i16(uint64_t(0x0001000000120034ull),
                        uint64_t(0xFFFEFFFF7FFF8000ull));
  const Dqword V1_v8i16(uint64_t(0x00057FF80081007Eull),
                        uint64_t(0x0088007700660055ull));
  const Dqword Expected_v8i16(uint64_t(0xFEFF7F8001001234ull),
                              uint64_t(0x7F776655057F7F7Eull));

#define TestImplXmmXmm(Dst, Src, Inst, Ty)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Inst ", " #Ty ")";                            \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst,                      \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImplXmmAddr(Dst, Inst, Ty)                                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, " #Inst ", " #Ty ")";                                \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplXmmXmm(Dst, Src, packss, v4i32);                                   \
    TestImplXmmAddr(Dst, packss, v4i32);                                       \
    TestImplXmmXmm(Dst, Src, packss, v8i16);                                   \
    TestImplXmmAddr(Dst, packss, v8i16);                                       \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplXmmAddr
#undef TestImplXmmXmm
}

TEST_F(AssemblerX8632Test, Packus) {
  const Dqword V0_v4i32(uint64_t(0x0001000000001234ull),
                        uint64_t(0x7FFFFFFF80000000ull));
  const Dqword V1_v4i32(uint64_t(0xFFFFFFFEFFFFFFFFull),
                        uint64_t(0x0000800100007FFEull));
  const Dqword Expected_v4i32(uint64_t(0xFFFF0000FFFF1234ull),
                              uint64_t(0x80017FFE00000000ull));

  const Dqword V0_v8i16(uint64_t(0x0001000000120034ull),
                        uint64_t(0xFFFEFFFF7FFF8000ull));
  const Dqword V1_v8i16(uint64_t(0x00057FF80081007Eull),
                        uint64_t(0x0088007700660055ull));
  const Dqword Expected_v8i16(uint64_t(0x0000FF0001001234ull),
                              uint64_t(0x8877665505FF817Eull));

#define TestImplXmmXmm(Dst, Src, Inst, Ty)                                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Inst ", " #Ty ")";                            \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst,                      \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImplXmmAddr(Dst, Inst, Ty)                                         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, " #Inst ", " #Ty ")";                                \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_##Ty, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0_##Ty);                                             \
    test.setDqwordTo(T1, V1_##Ty);                                             \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected_##Ty, test.Dst<Dqword>()) << TestString;                \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplXmmXmm(Dst, Src, packus, v4i32);                                   \
    TestImplXmmAddr(Dst, packus, v4i32);                                       \
    TestImplXmmXmm(Dst, Src, packus, v8i16);                                   \
    TestImplXmmAddr(Dst, packus, v8i16);                                       \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplXmmAddr
#undef TestImplXmmXmm
}

TEST_F(AssemblerX8632Test, Pshufb) {
  const Dqword V0(uint64_t(0x1122334455667788ull),
                  uint64_t(0x99aabbccddeeff32ull));
  const Dqword V1(uint64_t(0x0204050380060708ull),
                  uint64_t(0x010306080a8b0c0dull));

  const Dqword Expected(uint64_t(0x6644335500221132ull),
                        uint64_t(0x77552232ee00ccbbull));

#define TestImplXmmXmm(Dst, Src, Inst)                                         \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Inst ")";    \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_void, XmmRegister::Encoded_Reg_##Dst,                      \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestImplXmmAddr(Dst, Inst)                                             \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr, " #Inst ")";        \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_void, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplXmmXmm(Dst, Src, pshufb);                                          \
    TestImplXmmAddr(Dst, pshufb);                                              \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplXmmAddr
#undef TestImplXmmXmm
}

TEST_F(AssemblerX8632Test, Cvt) {
  const Dqword dq2ps32DstValue(-1.0f, -1.0f, -1.0f, -1.0f);
  const Dqword dq2ps32SrcValue(-5, 3, 100, 200);
  const Dqword dq2ps32Expected(-5.0f, 3.0f, 100.0, 200.0);

  const Dqword dq2ps64DstValue(0.0f, 0.0f, -1.0f, -1.0f);
  const Dqword dq2ps64SrcValue(-5, 3, 100, 200);
  const Dqword dq2ps64Expected(-5.0f, 3.0f, 100.0, 200.0);

  const Dqword tps2dq32DstValue(-1.0f, -1.0f, -1.0f, -1.0f);
  const Dqword tps2dq32SrcValue(-5.0f, 3.0f, 100.0, 200.0);
  const Dqword tps2dq32Expected(-5, 3, 100, 200);

  const Dqword tps2dq64DstValue(-1.0f, -1.0f, -1.0f, -1.0f);
  const Dqword tps2dq64SrcValue(-5.0f, 3.0f, 100.0, 200.0);
  const Dqword tps2dq64Expected(-5, 3, 100, 200);

  const Dqword si2ss32DstValue(-1.0f, -1.0f, -1.0f, -1.0f);
  const int32_t si2ss32SrcValue = 5;
  const Dqword si2ss32Expected(5.0f, -1.0f, -1.0f, -1.0f);

  const Dqword si2ss64DstValue(-1.0, -1.0);
  const int32_t si2ss64SrcValue = 5;
  const Dqword si2ss64Expected(5.0, -1.0);

  const int32_t tss2si32DstValue = 0xF00F0FF0;
  const Dqword tss2si32SrcValue(-5.0f, -1.0f, -1.0f, -1.0f);
  const int32_t tss2si32Expected = -5;

  const int32_t tss2si64DstValue = 0xF00F0FF0;
  const Dqword tss2si64SrcValue(-5.0, -1.0);
  const int32_t tss2si64Expected = -5;

  const Dqword float2float32DstValue(-1.0, -1.0);
  const Dqword float2float32SrcValue(-5.0, 3, 100, 200);
  const Dqword float2float32Expected(-5.0, -1.0);

  const Dqword float2float64DstValue(-1.0, -1.0, -1.0, -1.0);
  const Dqword float2float64SrcValue(-5.0, 3.0);
  const Dqword float2float64Expected(-5.0, -1.0, -1.0, -1.0);

#define TestImplPXmmXmm(Dst, Src, Inst, Size)                                  \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", cvt" #Inst ", f" #Size ")";                      \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ cvt##Inst(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,              \
                 XmmRegister::Encoded_Reg_##Src);                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##DstValue);                                \
    test.setDqwordTo(T1, Inst##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Size##Expected, test.Dst<Dqword>()) << TestString;         \
    reset();                                                                   \
  } while (0)

#define TestImplSXmmReg(Dst, GPR, Inst, Size)                                  \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #GPR ", cvt" #Inst ", f" #Size ")";                      \
    const uint32_t T0 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##GPR,                        \
           Immediate(Inst##Size##SrcValue));                                   \
    __ cvt##Inst(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst, IceType_i32, \
                 GPRRegister::Encoded_Reg_##GPR);                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##DstValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Size##Expected, test.Dst<Dqword>()) << TestString;         \
    reset();                                                                   \
  } while (0)

#define TestImplSRegXmm(GPR, Src, Inst, Size)                                  \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #GPR ", " #Src ", cvt" #Inst ", f" #Size ")";                      \
    const uint32_t T0 = allocateDqword();                                      \
                                                                               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##GPR,                        \
           Immediate(Inst##Size##DstValue));                                   \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ cvt##Inst(IceType_i32, GPRRegister::Encoded_Reg_##GPR, IceType_f##Size, \
                 XmmRegister::Encoded_Reg_##Src);                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Inst##Size##Expected), test.GPR())         \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplPXmmAddr(Dst, Inst, Size)                                      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, cvt" #Inst ", f" #Size ")";                          \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ cvt##Inst(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,              \
                 dwordAddress(T1));                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##DstValue);                                \
    test.setDqwordTo(T1, Inst##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Size##Expected, test.Dst<Dqword>()) << TestString;         \
    reset();                                                                   \
  } while (0)

#define TestImplSXmmAddr(Dst, Inst, Size)                                      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, cvt" #Inst ", f" #Size ")";                          \
    const uint32_t T0 = allocateDqword();                                      \
    const uint32_t T1 = allocateDword();                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ cvt##Inst(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst, IceType_i32, \
                 dwordAddress(T1));                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##DstValue);                                \
    test.setDwordTo(T1, Inst##Size##SrcValue);                                 \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Inst##Size##Expected, test.Dst<Dqword>()) << TestString;         \
    reset();                                                                   \
  } while (0)

#define TestImplSRegAddr(GPR, Inst, Size)                                      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #GPR ", Addr, cvt" #Inst ", f" #Size ")";                          \
    const uint32_t T0 = allocateDqword();                                      \
                                                                               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##GPR,                        \
           Immediate(Inst##Size##DstValue));                                   \
    __ cvt##Inst(IceType_i32, GPRRegister::Encoded_Reg_##GPR, IceType_f##Size, \
                 dwordAddress(T0));                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, Inst##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<uint32_t>(Inst##Size##Expected), test.GPR())         \
        << TestString;                                                         \
    reset();                                                                   \
  } while (0)

#define TestImplSize(Dst, Src, GPR, Size)                                      \
  do {                                                                         \
    TestImplPXmmXmm(Dst, Src, dq2ps, Size);                                    \
    TestImplPXmmAddr(Src, dq2ps, Size);                                        \
    TestImplPXmmXmm(Dst, Src, tps2dq, Size);                                   \
    TestImplPXmmAddr(Src, tps2dq, Size);                                       \
    TestImplSXmmReg(Dst, GPR, si2ss, Size);                                    \
    TestImplSXmmAddr(Dst, si2ss, Size);                                        \
    TestImplSRegXmm(GPR, Src, tss2si, Size);                                   \
    TestImplSRegAddr(GPR, tss2si, Size);                                       \
    TestImplPXmmXmm(Dst, Src, float2float, Size);                              \
    TestImplPXmmAddr(Src, float2float, Size);                                  \
  } while (0)

#define TestImpl(Dst, Src, GPR)                                                \
  do {                                                                         \
    TestImplSize(Dst, Src, GPR, 32);                                           \
    TestImplSize(Dst, Src, GPR, 64);                                           \
  } while (0)

  TestImpl(xmm0, xmm1, eax);
  TestImpl(xmm1, xmm2, ebx);
  TestImpl(xmm2, xmm3, ecx);
  TestImpl(xmm3, xmm4, edx);
  TestImpl(xmm4, xmm5, esi);
  TestImpl(xmm5, xmm6, edi);
  TestImpl(xmm6, xmm7, eax);
  TestImpl(xmm7, xmm0, ebx);

#undef TestImpl
#undef TestImplSize
#undef TestImplSRegAddr
#undef TestImplSXmmAddr
#undef TestImplPXmmAddr
#undef TestImplSRegXmm
#undef TestImplSXmmReg
#undef TestImplPXmmXmm
}

TEST_F(AssemblerX8632Test, Ucomiss) {
  static constexpr float qnan32 = std::numeric_limits<float>::quiet_NaN();
  static constexpr double qnan64 = std::numeric_limits<float>::quiet_NaN();

  Dqword test32DstValue(0.0, qnan32, qnan32, qnan32);
  Dqword test32SrcValue(0.0, qnan32, qnan32, qnan32);

  Dqword test64DstValue(0.0, qnan64);
  Dqword test64SrcValue(0.0, qnan64);

#define TestImplXmmXmm(Dst, Value0, Src, Value1, Size, CompType, BParity,      \
                       BOther)                                                 \
  do {                                                                         \
    static constexpr char NearBranch = AssemblerX8632::kNearJump;              \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Size ", " #CompType \
        ", " #BParity ", " #BOther ")";                                        \
    const uint32_t T0 = allocateDqword();                                      \
    test##Size##DstValue.F##Size[0] = Value0;                                  \
    const uint32_t T1 = allocateDqword();                                      \
    test##Size##SrcValue.F##Size[0] = Value1;                                  \
    const uint32_t ImmIfTrue = 0xBEEF;                                         \
    const uint32_t ImmIfFalse = 0xC0FFE;                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ImmIfFalse));  \
    __ ucomiss(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,                \
               XmmRegister::Encoded_Reg_##Src);                                \
    Label Done;                                                                \
    __ j(Cond::Br_##BParity, &Done, NearBranch);                               \
    __ j(Cond::Br_##BOther, &Done, NearBranch);                                \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ImmIfTrue));   \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, test##Size##DstValue);                                \
    test.setDqwordTo(T1, test##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(ImmIfTrue, test.eax()) << TestString;                            \
    reset();                                                                   \
  } while (0)

#define TestImplXmmAddr(Dst, Value0, Value1, Size, CompType, BParity, BOther)  \
  do {                                                                         \
    static constexpr char NearBranch = AssemblerX8632::kNearJump;              \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Size ", " #CompType     \
        ", " #BParity ", " #BOther ")";                                        \
    const uint32_t T0 = allocateDqword();                                      \
    test##Size##DstValue.F##Size[0] = Value0;                                  \
    const uint32_t T1 = allocateDqword();                                      \
    test##Size##SrcValue.F##Size[0] = Value1;                                  \
    const uint32_t ImmIfTrue = 0xBEEF;                                         \
    const uint32_t ImmIfFalse = 0xC0FFE;                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ImmIfFalse));  \
    __ ucomiss(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,                \
               dwordAddress(T1));                                              \
    Label Done;                                                                \
    __ j(Cond::Br_##BParity, &Done, NearBranch);                               \
    __ j(Cond::Br_##BOther, &Done, NearBranch);                                \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ImmIfTrue));   \
    __ bind(&Done);                                                            \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, test##Size##DstValue);                                \
    test.setDqwordTo(T1, test##Size##SrcValue);                                \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(ImmIfTrue, test.eax()) << TestString;                            \
    reset();                                                                   \
  } while (0)

#define TestImplCond(Dst, Value0, Src, Value1, Size, CompType, BParity,        \
                     BOther)                                                   \
  do {                                                                         \
    TestImplXmmXmm(Dst, Value0, Src, Value1, Size, CompType, BParity, BOther); \
    TestImplXmmAddr(Dst, Value0, Value1, Size, CompType, BParity, BOther);     \
  } while (0)

#define TestImplSize(Dst, Src, Size)                                           \
  do {                                                                         \
    TestImplCond(Dst, 1.0, Src, 1.0, Size, isEq, p, ne);                       \
    TestImplCond(Dst, 1.0, Src, 2.0, Size, isNe, p, e);                        \
    TestImplCond(Dst, 1.0, Src, 2.0, Size, isLe, p, a);                        \
    TestImplCond(Dst, 1.0, Src, 1.0, Size, isLe, p, a);                        \
    TestImplCond(Dst, 1.0, Src, 2.0, Size, isLt, p, ae);                       \
    TestImplCond(Dst, 2.0, Src, 1.0, Size, isGe, p, b);                        \
    TestImplCond(Dst, 1.0, Src, 1.0, Size, isGe, p, b);                        \
    TestImplCond(Dst, 2.0, Src, 1.0, Size, isGt, p, be);                       \
    TestImplCond(Dst, qnan##Size, Src, 1.0, Size, isUnord, np, o);             \
    TestImplCond(Dst, 1.0, Src, qnan##Size, Size, isUnord, np, s);             \
    TestImplCond(Dst, qnan##Size, Src, qnan##Size, Size, isUnord, np, s);      \
  } while (0)

#define TestImpl(Dst, Src)                                                     \
  do {                                                                         \
    TestImplSize(Dst, Src, 32);                                                \
    TestImplSize(Dst, Src, 64);                                                \
  } while (0)

  TestImpl(xmm0, xmm1);
  TestImpl(xmm1, xmm2);
  TestImpl(xmm2, xmm3);
  TestImpl(xmm3, xmm4);
  TestImpl(xmm4, xmm5);
  TestImpl(xmm5, xmm6);
  TestImpl(xmm6, xmm7);
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplSize
#undef TestImplCond
#undef TestImplXmmAddr
#undef TestImplXmmXmm
}

TEST_F(AssemblerX8632Test, Sqrtss) {
  Dqword test32SrcValue(-100.0, -100.0, -100.0, -100.0);
  Dqword test32DstValue(-1.0, -1.0, -1.0, -1.0);

  Dqword test64SrcValue(-100.0, -100.0);
  Dqword test64DstValue(-1.0, -1.0);

#define TestSqrtssXmmXmm(Dst, Src, Value1, Result, Size)                       \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Value1 ", " #Result ", " #Size ")";           \
    const uint32_t T0 = allocateDqword();                                      \
    test##Size##SrcValue.F##Size[0] = Value1;                                  \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));               \
    __ sqrt(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, test##Size##SrcValue);                                \
    test.setDqwordTo(T1, test##Size##DstValue);                                \
    test.run();                                                                \
                                                                               \
    Dqword Expected = test##Size##DstValue;                                    \
    Expected.F##Size[0] = Result;                                              \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestSqrtssXmmAddr(Dst, Value1, Result, Size)                           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", Addr, " #Value1 ", " #Result ", " #Size ")";               \
    const uint32_t T0 = allocateDqword();                                      \
    test##Size##SrcValue.F##Size[0] = Value1;                                  \
    const uint32_t T1 = allocateDqword();                                      \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));               \
    __ sqrt(IceType_f##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T0));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, test##Size##SrcValue);                                \
    test.setDqwordTo(T1, test##Size##DstValue);                                \
    test.run();                                                                \
                                                                               \
    Dqword Expected = test##Size##DstValue;                                    \
    Expected.F##Size[0] = Result;                                              \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestSqrtssSize(Dst, Src, Size)                                         \
  do {                                                                         \
    TestSqrtssXmmXmm(Dst, Src, 4.0, 2.0, Size);                                \
    TestSqrtssXmmAddr(Dst, 4.0, 2.0, Size);                                    \
    TestSqrtssXmmXmm(Dst, Src, 9.0, 3.0, Size);                                \
    TestSqrtssXmmAddr(Dst, 9.0, 3.0, Size);                                    \
    TestSqrtssXmmXmm(Dst, Src, 100.0, 10.0, Size);                             \
    TestSqrtssXmmAddr(Dst, 100.0, 10.0, Size);                                 \
  } while (0)

#define TestSqrtss(Dst, Src)                                                   \
  do {                                                                         \
    TestSqrtssSize(Dst, Src, 32);                                              \
    TestSqrtssSize(Dst, Src, 64);                                              \
  } while (0)

  TestSqrtss(xmm0, xmm1);
  TestSqrtss(xmm1, xmm2);
  TestSqrtss(xmm2, xmm3);
  TestSqrtss(xmm3, xmm4);
  TestSqrtss(xmm4, xmm5);
  TestSqrtss(xmm5, xmm6);
  TestSqrtss(xmm6, xmm7);
  TestSqrtss(xmm7, xmm0);

#undef TestSqrtss
#undef TestSqrtssSize
#undef TestSqrtssXmmAddr
#undef TestSqrtssXmmXmm
}

TEST_F(AssemblerX8632Test, Insertps) {
#define TestInsertpsXmmXmmImm(Dst, Value0, Src, Value1, Imm, Expected)         \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Imm ", " #Expected  \
        ")";                                                                   \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ insertps(IceType_v4f32, XmmRegister::Encoded_Reg_##Dst,                 \
                XmmRegister::Encoded_Reg_##Src, Immediate(Imm));               \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Dqword Expected, test.Dst<Dqword>()) << TestString;              \
    reset();                                                                   \
  } while (0)

#define TestInsertpsXmmAddrImm(Dst, Value0, Value1, Imm, Expected)             \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Imm ", " #Expected ")"; \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ insertps(IceType_v4f32, XmmRegister::Encoded_Reg_##Dst,                 \
                dwordAddress(T1), Immediate(Imm));                             \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(Dqword Expected, test.Dst<Dqword>()) << TestString;              \
    reset();                                                                   \
  } while (0)

#define TestInsertps(Dst, Src)                                                 \
  do {                                                                         \
    TestInsertpsXmmXmmImm(                                                     \
        Dst, (uint64_t(-1), uint64_t(-1)), Src,                                \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xCCCCCCCCDDDDDDDDull)),    \
        0x99,                                                                  \
        (uint64_t(0xDDDDDDDD00000000ull), uint64_t(0x00000000FFFFFFFFull)));   \
    TestInsertpsXmmAddrImm(                                                    \
        Dst, (uint64_t(-1), uint64_t(-1)),                                     \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xCCCCCCCCDDDDDDDDull)),    \
        0x99,                                                                  \
        (uint64_t(0xBBBBBBBB00000000ull), uint64_t(0x00000000FFFFFFFFull)));   \
    TestInsertpsXmmXmmImm(                                                     \
        Dst, (uint64_t(-1), uint64_t(-1)), Src,                                \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xCCCCCCCCDDDDDDDDull)),    \
        0x9D,                                                                  \
        (uint64_t(0xDDDDDDDD00000000ull), uint64_t(0x0000000000000000ull)));   \
    TestInsertpsXmmAddrImm(                                                    \
        Dst, (uint64_t(-1), uint64_t(-1)),                                     \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xCCCCCCCCDDDDDDDDull)),    \
        0x9D,                                                                  \
        (uint64_t(0xBBBBBBBB00000000ull), uint64_t(0x0000000000000000ull)));   \
  } while (0)

  TestInsertps(xmm0, xmm1);
  TestInsertps(xmm1, xmm2);
  TestInsertps(xmm2, xmm3);
  TestInsertps(xmm3, xmm4);
  TestInsertps(xmm4, xmm5);
  TestInsertps(xmm5, xmm6);
  TestInsertps(xmm6, xmm7);
  TestInsertps(xmm7, xmm0);

#undef TestInsertps
#undef TestInsertpsXmmXmmAddr
#undef TestInsertpsXmmXmmImm
}

TEST_F(AssemblerX8632Test, Pinsr) {
  static constexpr uint8_t Mask32 = 0x03;
  static constexpr uint8_t Mask16 = 0x07;
  static constexpr uint8_t Mask8 = 0x0F;

#define TestPinsrXmmGPRImm(Dst, Value0, GPR, Value1, Imm, Size)                \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #GPR ", " #Value1 ", " #Imm ", " #Size ")"; \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##GPR, Immediate(Value1));    \
    __ pinsr(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                  \
             GPRRegister::Encoded_Reg_##GPR, Immediate(Imm));                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
                                                                               \
    constexpr uint8_t sel = (Imm)&Mask##Size;                                  \
    Dqword Expected = V0;                                                      \
    Expected.U##Size[sel] = Value1;                                            \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestPinsrXmmAddrImm(Dst, Value0, Value1, Imm, Size)                    \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Imm ", " #Size ")";     \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = Value1;                                                \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ pinsr(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                  \
             dwordAddress(T1), Immediate(Imm));                                \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDwordTo(T1, V1);                                                   \
    test.run();                                                                \
                                                                               \
    constexpr uint8_t sel = (Imm)&Mask##Size;                                  \
    Dqword Expected = V0;                                                      \
    Expected.U##Size[sel] = Value1;                                            \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestPinsrSize(Dst, GPR, Value1, Imm, Size)                             \
  do {                                                                         \
    TestPinsrXmmGPRImm(                                                        \
        Dst,                                                                   \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xFFFFFFFFDDDDDDDDull)),    \
        GPR, Value1, Imm, Size);                                               \
    TestPinsrXmmAddrImm(                                                       \
        Dst,                                                                   \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xFFFFFFFFDDDDDDDDull)),    \
        Value1, Imm, Size);                                                    \
  } while (0)

#define TestPinsr(Src, Dst)                                                    \
  do {                                                                         \
    TestPinsrSize(Src, Dst, 0xEE, 0x03, 8);                                    \
    TestPinsrSize(Src, Dst, 0xFFEE, 0x03, 16);                                 \
    TestPinsrSize(Src, Dst, 0xC0FFEE, 0x03, 32);                               \
  } while (0)

  TestPinsr(xmm0, eax);
  TestPinsr(xmm1, ebx);
  TestPinsr(xmm2, ecx);
  TestPinsr(xmm3, edx);
  TestPinsr(xmm4, esi);
  TestPinsr(xmm5, edi);
  TestPinsr(xmm6, eax);
  TestPinsr(xmm7, ebx);

#undef TestPinsr
#undef TestPinsrSize
#undef TestPinsrXmmAddrImm
#undef TestPinsrXmmGPRImm
}

TEST_F(AssemblerX8632Test, Pextr) {
  static constexpr uint8_t Mask32 = 0x03;
  static constexpr uint8_t Mask16 = 0x07;
  static constexpr uint8_t Mask8 = 0x0F;

#define TestPextrGPRXmmImm(GPR, Src, Value1, Imm, Size)                        \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #GPR ", " #Src ", " #Value1 ", " #Imm ", " #Size ")";              \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ pextr(IceType_i##Size, GPRRegister::Encoded_Reg_##GPR,                  \
             XmmRegister::Encoded_Reg_##Src, Immediate(Imm));                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.run();                                                                \
                                                                               \
    constexpr uint8_t sel = (Imm)&Mask##Size;                                  \
    ASSERT_EQ(V0.U##Size[sel], test.GPR()) << TestString;                      \
    reset();                                                                   \
  } while (0)

#define TestPextrSize(GPR, Src, Value1, Imm, Size)                             \
  do {                                                                         \
    TestPextrGPRXmmImm(                                                        \
        GPR, Src,                                                              \
        (uint64_t(0xAAAAAAAABBBBBBBBull), uint64_t(0xFFFFFFFFDDDDDDDDull)),    \
        Imm, Size);                                                            \
  } while (0)

#define TestPextr(Src, Dst)                                                    \
  do {                                                                         \
    TestPextrSize(Src, Dst, 0xEE, 0x03, 8);                                    \
    TestPextrSize(Src, Dst, 0xFFEE, 0x03, 16);                                 \
    TestPextrSize(Src, Dst, 0xC0FFEE, 0x03, 32);                               \
  } while (0)

  TestPextr(eax, xmm0);
  TestPextr(ebx, xmm1);
  TestPextr(ecx, xmm2);
  TestPextr(edx, xmm3);
  TestPextr(esi, xmm4);
  TestPextr(edi, xmm5);
  TestPextr(eax, xmm6);
  TestPextr(ebx, xmm7);

#undef TestPextr
#undef TestPextrSize
#undef TestPextrXmmGPRImm
}

TEST_F(AssemblerX8632Test, Pcmpeq_Pcmpgt) {
#define TestPcmpXmmXmm(Dst, Value0, Src, Value1, Size, Inst, Op)               \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", " #Src ", " #Value1 ", " #Size ", " #Op ")";  \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            XmmRegister::Encoded_Reg_##Src);                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    Dqword Expected(uint64_t(0), uint64_t(0));                                 \
    static constexpr uint8_t ArraySize =                                       \
        sizeof(Dqword) / sizeof(uint##Size##_t);                               \
    for (uint8_t i = 0; i < ArraySize; ++i) {                                  \
      Expected.I##Size[i] = (V1.I##Size[i] Op V0.I##Size[i]) ? -1 : 0;         \
    }                                                                          \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestPcmpXmmAddr(Dst, Value0, Value1, Size, Inst, Op)                   \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Value0 ", Addr, " #Value1 ", " #Size ", " #Op ")";      \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value0;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ Inst(IceType_i##Size, XmmRegister::Encoded_Reg_##Dst,                   \
            dwordAddress(T1));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    Dqword Expected(uint64_t(0), uint64_t(0));                                 \
    static constexpr uint8_t ArraySize =                                       \
        sizeof(Dqword) / sizeof(uint##Size##_t);                               \
    for (uint8_t i = 0; i < ArraySize; ++i) {                                  \
      Expected.I##Size[i] = (V1.I##Size[i] Op V0.I##Size[i]) ? -1 : 0;         \
    }                                                                          \
    ASSERT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestPcmpValues(Dst, Value0, Src, Value1, Size)                         \
  do {                                                                         \
    TestPcmpXmmXmm(Dst, Value0, Src, Value1, Size, pcmpeq, ==);                \
    TestPcmpXmmAddr(Dst, Value0, Value1, Size, pcmpeq, ==);                    \
    TestPcmpXmmXmm(Dst, Value0, Src, Value1, Size, pcmpgt, <);                 \
    TestPcmpXmmAddr(Dst, Value0, Value1, Size, pcmpgt, <);                     \
  } while (0)

#define TestPcmpSize(Dst, Src, Size)                                           \
  do {                                                                         \
    TestPcmpValues(                                                            \
        Dst,                                                                   \
        (uint64_t(0x8888888888888888ull), uint64_t(0x0000000000000000ull)),    \
        Src,                                                                   \
        (uint64_t(0x0000008800008800ull), uint64_t(0xFFFFFFFFFFFFFFFFull)),    \
        Size);                                                                 \
    TestPcmpValues(                                                            \
        Dst,                                                                   \
        (uint64_t(0x123567ABAB55DE01ull), uint64_t(0x12345abcde12345Aull)),    \
        Src,                                                                   \
        (uint64_t(0x0000008800008800ull), uint64_t(0xAABBCCDD1234321Aull)),    \
        Size);                                                                 \
  } while (0)

#define TestPcmp(Dst, Src)                                                     \
  do {                                                                         \
    TestPcmpSize(xmm0, xmm1, 8);                                               \
    TestPcmpSize(xmm0, xmm1, 16);                                              \
    TestPcmpSize(xmm0, xmm1, 32);                                              \
  } while (0)

  TestPcmp(xmm0, xmm1);
  TestPcmp(xmm1, xmm2);
  TestPcmp(xmm2, xmm3);
  TestPcmp(xmm3, xmm4);
  TestPcmp(xmm4, xmm5);
  TestPcmp(xmm5, xmm6);
  TestPcmp(xmm6, xmm7);
  TestPcmp(xmm7, xmm0);

#undef TestPcmp
#undef TestPcmpSize
#undef TestPcmpValues
#undef TestPcmpXmmAddr
#undef TestPcmpXmmXmm
}

TEST_F(AssemblerX8632Test, Roundsd) {
#define TestRoundsdXmmXmm(Dst, Src, Mode, Input, RN)                           \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Dst ", " #Src ", " #Mode ", " #Input ", " #RN ")";                \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(-3.0, -3.0);                                               \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(double(Input), -123.4);                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ round(IceType_f64, XmmRegister::Encoded_Reg_##Dst,                      \
             XmmRegister::Encoded_Reg_##Src,                                   \
             Immediate(AssemblerX8632::k##Mode));                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDqwordTo(T0, V0);                                                  \
    test.setDqwordTo(T1, V1);                                                  \
    test.run();                                                                \
                                                                               \
    const Dqword Expected(double(RN), -3.0);                                   \
    EXPECT_EQ(Expected, test.Dst<Dqword>()) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestRoundsd(Dst, Src)                                                  \
  do {                                                                         \
    TestRoundsdXmmXmm(Dst, Src, RoundToNearest, 5.51, 6);                      \
    TestRoundsdXmmXmm(Dst, Src, RoundToNearest, 5.49, 5);                      \
    TestRoundsdXmmXmm(Dst, Src, RoundDown, 5.51, 5);                           \
    TestRoundsdXmmXmm(Dst, Src, RoundUp, 5.49, 6);                             \
    TestRoundsdXmmXmm(Dst, Src, RoundToZero, 5.49, 5);                         \
    TestRoundsdXmmXmm(Dst, Src, RoundToZero, 5.51, 5);                         \
  } while (0)

  TestRoundsd(xmm0, xmm1);
  TestRoundsd(xmm1, xmm2);
  TestRoundsd(xmm2, xmm3);
  TestRoundsd(xmm3, xmm4);
  TestRoundsd(xmm4, xmm5);
  TestRoundsd(xmm5, xmm6);
  TestRoundsd(xmm6, xmm7);
  TestRoundsd(xmm7, xmm0);

#undef TestRoundsd
#undef TestRoundsdXmmXmm
}

TEST_F(AssemblerX8632Test, Set1ps) {
#define TestImpl(Xmm, Src, Imm)                                                \
  do {                                                                         \
    __ set1ps(XmmRegister::Encoded_Reg_##Xmm, GPRRegister::Encoded_Reg_##Src,  \
              Immediate(Imm));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    const Dqword Expected((uint64_t(Imm) << 32) | uint32_t(Imm),               \
                          (uint64_t(Imm) << 32) | uint32_t(Imm));              \
    ASSERT_EQ(Expected, test.Xmm<Dqword>())                                    \
        << "(" #Xmm ", " #Src ", " #Imm ")";                                   \
    reset();                                                                   \
  } while (0)

  TestImpl(xmm0, ebx, 1);
  TestImpl(xmm1, ecx, 2);
  TestImpl(xmm2, edx, 3);
  TestImpl(xmm3, esi, 4);
  TestImpl(xmm4, edi, 5);
  TestImpl(xmm5, eax, 6);
  TestImpl(xmm6, ebx, 7);
  TestImpl(xmm7, ecx, 8);

#undef TestImpl
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice
