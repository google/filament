//===- subzero/unittest/AssemblerX8632/DataMov.cpp ------------------------===//
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

TEST_F(AssemblerX8632Test, MovRegImm) {
  constexpr uint32_t ExpectedEax = 0x000000FFul;
  constexpr uint32_t ExpectedEbx = 0x0000FF00ul;
  constexpr uint32_t ExpectedEcx = 0x00FF0000ul;
  constexpr uint32_t ExpectedEdx = 0xFF000000ul;
  constexpr uint32_t ExpectedEdi = 0x6AAA0006ul;
  constexpr uint32_t ExpectedEsi = 0x6000AAA6ul;

  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ExpectedEax));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ebx, Immediate(ExpectedEbx));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ecx, Immediate(ExpectedEcx));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edx, Immediate(ExpectedEdx));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edi, Immediate(ExpectedEdi));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi, Immediate(ExpectedEsi));

  AssembledTest test = assemble();
  test.run();
  EXPECT_EQ(ExpectedEax, test.eax());
  EXPECT_EQ(ExpectedEbx, test.ebx());
  EXPECT_EQ(ExpectedEcx, test.ecx());
  EXPECT_EQ(ExpectedEdx, test.edx());
  EXPECT_EQ(ExpectedEdi, test.edi());
  EXPECT_EQ(ExpectedEsi, test.esi());
}

TEST_F(AssemblerX8632Test, MovMemImm) {
  const uint32_t T0 = allocateDword();
  constexpr uint32_t ExpectedT0 = 0x00111100ul;
  const uint32_t T1 = allocateDword();
  constexpr uint32_t ExpectedT1 = 0x00222200ul;
  const uint32_t T2 = allocateDword();
  constexpr uint32_t ExpectedT2 = 0x03333000ul;
  const uint32_t T3 = allocateDword();
  constexpr uint32_t ExpectedT3 = 0x00444400ul;

  __ mov(IceType_i32, dwordAddress(T0), Immediate(ExpectedT0));
  __ mov(IceType_i32, dwordAddress(T1), Immediate(ExpectedT1));
  __ mov(IceType_i32, dwordAddress(T2), Immediate(ExpectedT2));
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
  EXPECT_EQ(ExpectedT1, test.contentsOfDword(T1));
  EXPECT_EQ(ExpectedT2, test.contentsOfDword(T2));
  EXPECT_EQ(ExpectedT3, test.contentsOfDword(T3));
}

TEST_F(AssemblerX8632Test, MovMemReg) {
  const uint32_t T0 = allocateDword();
  constexpr uint32_t ExpectedT0 = 0x00111100ul;
  const uint32_t T1 = allocateDword();
  constexpr uint32_t ExpectedT1 = 0x00222200ul;
  const uint32_t T2 = allocateDword();
  constexpr uint32_t ExpectedT2 = 0x00333300ul;
  const uint32_t T3 = allocateDword();
  constexpr uint32_t ExpectedT3 = 0x00444400ul;
  const uint32_t T4 = allocateDword();
  constexpr uint32_t ExpectedT4 = 0x00555500ul;
  const uint32_t T5 = allocateDword();
  constexpr uint32_t ExpectedT5 = 0x00666600ul;

  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(ExpectedT0));
  __ mov(IceType_i32, dwordAddress(T0), GPRRegister::Encoded_Reg_eax);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ebx, Immediate(ExpectedT1));
  __ mov(IceType_i32, dwordAddress(T1), GPRRegister::Encoded_Reg_ebx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ecx, Immediate(ExpectedT2));
  __ mov(IceType_i32, dwordAddress(T2), GPRRegister::Encoded_Reg_ecx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edx, Immediate(ExpectedT3));
  __ mov(IceType_i32, dwordAddress(T3), GPRRegister::Encoded_Reg_edx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edi, Immediate(ExpectedT4));
  __ mov(IceType_i32, dwordAddress(T4), GPRRegister::Encoded_Reg_edi);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi, Immediate(ExpectedT5));
  __ mov(IceType_i32, dwordAddress(T5), GPRRegister::Encoded_Reg_esi);

  AssembledTest test = assemble();
  test.run();
  EXPECT_EQ(ExpectedT0, test.contentsOfDword(T0));
  EXPECT_EQ(ExpectedT1, test.contentsOfDword(T1));
  EXPECT_EQ(ExpectedT2, test.contentsOfDword(T2));
  EXPECT_EQ(ExpectedT3, test.contentsOfDword(T3));
  EXPECT_EQ(ExpectedT4, test.contentsOfDword(T4));
  EXPECT_EQ(ExpectedT5, test.contentsOfDword(T5));
}

TEST_F(AssemblerX8632Test, MovRegReg) {
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0x20));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ebx,
         GPRRegister::Encoded_Reg_eax);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ecx,
         GPRRegister::Encoded_Reg_ebx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edx,
         GPRRegister::Encoded_Reg_ecx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edi,
         GPRRegister::Encoded_Reg_edx);
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi,
         GPRRegister::Encoded_Reg_edi);

  __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi, Immediate(0x55000000ul));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax,
         GPRRegister::Encoded_Reg_esi);

  AssembledTest test = assemble();
  test.run();
  EXPECT_EQ(0x55000000ul, test.eax());
  EXPECT_EQ(0x20ul, test.ebx());
  EXPECT_EQ(0x20ul, test.ecx());
  EXPECT_EQ(0x20ul, test.edx());
  EXPECT_EQ(0x20ul, test.edi());
  EXPECT_EQ(0x55000000ul, test.esi());
}

TEST_F(AssemblerX8632Test, MovRegMem) {
  const uint32_t T0 = allocateDword();
  constexpr uint32_t ExpectedT0 = 0x00111100ul;
  const uint32_t T1 = allocateDword();
  constexpr uint32_t ExpectedT1 = 0x00222200ul;
  const uint32_t T2 = allocateDword();
  constexpr uint32_t ExpectedT2 = 0x00333300ul;
  const uint32_t T3 = allocateDword();
  constexpr uint32_t ExpectedT3 = 0x00444400ul;
  const uint32_t T4 = allocateDword();
  constexpr uint32_t ExpectedT4 = 0x00555500ul;
  const uint32_t T5 = allocateDword();
  constexpr uint32_t ExpectedT5 = 0x00666600ul;

  __ mov(IceType_i32, dwordAddress(T0), Immediate(ExpectedT0));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, dwordAddress(T0));

  __ mov(IceType_i32, dwordAddress(T1), Immediate(ExpectedT1));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ebx, dwordAddress(T1));

  __ mov(IceType_i32, dwordAddress(T2), Immediate(ExpectedT2));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_ecx, dwordAddress(T2));

  __ mov(IceType_i32, dwordAddress(T3), Immediate(ExpectedT3));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edx, dwordAddress(T3));

  __ mov(IceType_i32, dwordAddress(T4), Immediate(ExpectedT4));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_edi, dwordAddress(T4));

  __ mov(IceType_i32, dwordAddress(T5), Immediate(ExpectedT5));
  __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi, dwordAddress(T5));

  AssembledTest test = assemble();
  test.run();
  EXPECT_EQ(ExpectedT0, test.eax());
  EXPECT_EQ(ExpectedT1, test.ebx());
  EXPECT_EQ(ExpectedT2, test.ecx());
  EXPECT_EQ(ExpectedT3, test.edx());
  EXPECT_EQ(ExpectedT4, test.edi());
  EXPECT_EQ(ExpectedT5, test.esi());
}

TEST_F(AssemblerX8632Test, Movzx) {
#define TestMovzx8bitWithRegDest(Src, Dst, Imm)                                \
  do {                                                                         \
    static_assert(((Imm)&0xFF) == (Imm), #Imm " is not an 8bit immediate");    \
    __ mov(IceType_i8, GPRRegister::Encoded_Reg_##Src, Immediate(Imm));        \
    __ movzx(IceType_i8, GPRRegister::Encoded_Reg_##Dst,                       \
             GPRRegister::Encoded_Reg_##Src);                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ(Imm, test.Dst()) << "(" #Src ", " #Dst ", " #Imm ")";            \
    reset();                                                                   \
  } while (0)

#define TestMovzx16bitWithRegDest(Src, Dst, Imm)                               \
  do {                                                                         \
    static_assert(((Imm)&0xFFFF) == (Imm), #Imm " is not a 16bit immediate");  \
    __ mov(IceType_i16, GPRRegister::Encoded_Reg_##Src, Immediate(Imm));       \
    __ movzx(IceType_i16, GPRRegister::Encoded_Reg_##Dst,                      \
             GPRRegister::Encoded_Reg_##Src);                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ(Imm, test.Dst()) << "(" #Src ", " #Dst ", " #Imm ")";            \
    reset();                                                                   \
  } while (0)

#define TestMovzx8bitWithAddrSrc(Dst, Imm)                                     \
  do {                                                                         \
    static_assert(((Imm)&0xFF) == (Imm), #Imm " is not an 8bit immediate");    \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Imm;                                                   \
    __ movzx(IceType_i8, GPRRegister::Encoded_Reg_##Dst, dwordAddress(T0));    \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ(Imm, test.Dst()) << "(Addr, " #Dst ", " #Imm ")";                \
    reset();                                                                   \
  } while (0)

#define TestMovzx16bitWithAddrSrc(Dst, Imm)                                    \
  do {                                                                         \
    static_assert(((Imm)&0xFFFF) == (Imm), #Imm " is not a 16bit immediate");  \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Imm;                                                   \
    __ movzx(IceType_i16, GPRRegister::Encoded_Reg_##Dst, dwordAddress(T0));   \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ(Imm, test.Dst()) << "(Addr, " #Dst ", " #Imm ")";                \
    reset();                                                                   \
  } while (0)

#define TestMovzx(Dst)                                                         \
  do {                                                                         \
    TestMovzx8bitWithRegDest(eax, Dst, 0x81u);                                 \
    TestMovzx8bitWithRegDest(ebx, Dst, 0x82u);                                 \
    TestMovzx8bitWithRegDest(ecx, Dst, 0x83u);                                 \
    TestMovzx8bitWithRegDest(edx, Dst, 0x84u);                                 \
    /* esi is encoded as dh */                                                 \
    TestMovzx8bitWithRegDest(esi, Dst, 0x85u);                                 \
    /* edi is encoded as bh */                                                 \
    TestMovzx8bitWithRegDest(edi, Dst, 0x86u);                                 \
    /* ebp is encoded as ch */                                                 \
    TestMovzx8bitWithRegDest(ebp, Dst, 0x87u);                                 \
    /* esp is encoded as ah */                                                 \
    TestMovzx8bitWithRegDest(esp, Dst, 0x88u);                                 \
    TestMovzx8bitWithAddrSrc(Dst, 0x8Fu);                                      \
                                                                               \
    TestMovzx16bitWithRegDest(eax, Dst, 0x8118u);                              \
    TestMovzx16bitWithRegDest(ebx, Dst, 0x8228u);                              \
    TestMovzx16bitWithRegDest(ecx, Dst, 0x8338u);                              \
    TestMovzx16bitWithRegDest(edx, Dst, 0x8448u);                              \
    TestMovzx16bitWithAddrSrc(Dst, 0x8FF8u);                                   \
  } while (0)

  TestMovzx(eax);
  TestMovzx(ebx);
  TestMovzx(ecx);
  TestMovzx(edx);
  TestMovzx(esi);
  TestMovzx(edi);

#undef TestMovzx
#undef TestMovzx16bitWithAddrDest
#undef TestMovzx8bitWithAddrDest
#undef TestMovzx16bitWithRegDest
#undef TestMovzx8bitWithRegDest
}

TEST_F(AssemblerX8632Test, Movsx) {
#define TestMovsx8bitWithRegDest(Src, Dst, Imm)                                \
  do {                                                                         \
    static_assert(((Imm)&0xFF) == (Imm), #Imm " is not an 8bit immediate");    \
    __ mov(IceType_i8, GPRRegister::Encoded_Reg_##Src, Immediate(Imm));        \
    __ movsx(IceType_i8, GPRRegister::Encoded_Reg_##Dst,                       \
             GPRRegister::Encoded_Reg_##Src);                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((0xFFFFFF00 | (Imm)), test.Dst())                                \
        << "(" #Src ", " #Dst ", " #Imm ")";                                   \
    reset();                                                                   \
  } while (0)

#define TestMovsx16bitWithRegDest(Src, Dst, Imm)                               \
  do {                                                                         \
    static_assert(((Imm)&0xFFFF) == (Imm), #Imm " is not a 16bit immediate");  \
    __ mov(IceType_i16, GPRRegister::Encoded_Reg_##Src, Immediate(Imm));       \
    __ movsx(IceType_i16, GPRRegister::Encoded_Reg_##Dst,                      \
             GPRRegister::Encoded_Reg_##Src);                                  \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((0xFFFF0000 | (Imm)), test.Dst())                                \
        << "(" #Src ", " #Dst ", " #Imm ")";                                   \
    reset();                                                                   \
  } while (0)

#define TestMovsx8bitWithAddrSrc(Dst, Imm)                                     \
  do {                                                                         \
    static_assert(((Imm)&0xFF) == (Imm), #Imm " is not an 8bit immediate");    \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Imm;                                                   \
    __ movsx(IceType_i8, GPRRegister::Encoded_Reg_##Dst, dwordAddress(T0));    \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((0xFFFFFF00 | (Imm)), test.Dst())                                \
        << "(Addr, " #Dst ", " #Imm ")";                                       \
    reset();                                                                   \
  } while (0)

#define TestMovsx16bitWithAddrSrc(Dst, Imm)                                    \
  do {                                                                         \
    static_assert(((Imm)&0xFFFF) == (Imm), #Imm " is not a 16bit immediate");  \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Imm;                                                   \
    __ movsx(IceType_i16, GPRRegister::Encoded_Reg_##Dst, dwordAddress(T0));   \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((0xFFFF0000 | (Imm)), test.Dst())                                \
        << "(Addr, " #Dst ", " #Imm ")";                                       \
    reset();                                                                   \
  } while (0)

#define TestMovsx(Dst)                                                         \
  do {                                                                         \
    TestMovsx8bitWithRegDest(eax, Dst, 0x81u);                                 \
    TestMovsx8bitWithRegDest(ebx, Dst, 0x82u);                                 \
    TestMovsx8bitWithRegDest(ecx, Dst, 0x83u);                                 \
    TestMovsx8bitWithRegDest(edx, Dst, 0x84u);                                 \
    /* esi is encoded as dh */                                                 \
    TestMovsx8bitWithRegDest(esi, Dst, 0x85u);                                 \
    /* edi is encoded as bh */                                                 \
    TestMovsx8bitWithRegDest(edi, Dst, 0x86u);                                 \
    /* ebp is encoded as ch */                                                 \
    TestMovsx8bitWithRegDest(ebp, Dst, 0x87u);                                 \
    /* esp is encoded as ah */                                                 \
    TestMovsx8bitWithRegDest(esp, Dst, 0x88u);                                 \
    TestMovsx8bitWithAddrSrc(Dst, 0x8Fu);                                      \
                                                                               \
    TestMovsx16bitWithRegDest(eax, Dst, 0x8118u);                              \
    TestMovsx16bitWithRegDest(ebx, Dst, 0x8228u);                              \
    TestMovsx16bitWithRegDest(ecx, Dst, 0x8338u);                              \
    TestMovsx16bitWithRegDest(edx, Dst, 0x8448u);                              \
    TestMovsx16bitWithAddrSrc(Dst, 0x8FF8u);                                   \
  } while (0)

  TestMovsx(eax);
  TestMovsx(ebx);
  TestMovsx(ecx);
  TestMovsx(edx);
  TestMovsx(esi);
  TestMovsx(edi);

#undef TestMovsx
#undef TestMovsx16bitWithAddrDest
#undef TestMovsx8bitWithAddrDest
#undef TestMovsx16bitWithRegDest
#undef TestMovsx8bitWithRegDest
}

TEST_F(AssemblerX8632LowLevelTest, RepMovsb) {
  __ rep_movsb();

  static constexpr uint32_t ByteCount = 2;
  static constexpr uint8_t Prefix = 0xF3;
  static constexpr uint8_t Opcode = 0xA4;

  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), Prefix, Opcode);
}

TEST_F(AssemblerX8632Test, MovssXmmAddr) {
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
    __ movss(IceType_f##FloatLength, XmmRegister::Encoded_Reg_##Xmm,           \
             dwordAddress(T0));                                                \
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
    }                                                                          \
  } while (0)

  TestMovssXmmAddr(32);
  TestMovssXmmAddr(64);

#undef TestMovssXmmAddr
#undef TestMovssXmmAddrType
}

TEST_F(AssemblerX8632Test, MovssAddrXmm) {
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
    __ movss(IceType_f##FloatLength, XmmRegister::Encoded_Reg_##Xmm,           \
             dwordAddress(T0));                                                \
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
    }                                                                          \
  } while (0)

  TestMovssAddrXmm(32);
  TestMovssAddrXmm(64);

#undef TestMovssAddrXmm
#undef TestMovssAddrXmmType
}

TEST_F(AssemblerX8632Test, MovssXmmXmm) {
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
    __ movss(IceType_f##FloatLength, XmmRegister::Encoded_Reg_##Src,           \
             dwordAddress(T0));                                                \
    __ movss(IceType_f##FloatLength, XmmRegister::Encoded_Reg_##Dst,           \
             dwordAddress(T1));                                                \
    __ movss(IceType_f##FloatLength, XmmRegister::Encoded_Reg_##Dst,           \
             XmmRegister::Encoded_Reg_##Src);                                  \
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
      TestMovssXmmXmmFloatLength(FloatLength, xmm7, xmm0, Value);              \
    }                                                                          \
  } while (0)

  TestMovssXmmXmm(32);
  TestMovssXmmXmm(64);

#undef TestMovssXmmXmm
#undef TestMovssXmmXmmType
}

TEST_F(AssemblerX8632Test, MovdToXmm) {
#define TestMovdXmmReg(Src, Dst, Value)                                        \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = 0xFFFFFFFF00000000ull;                                 \
                                                                               \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src, Immediate(Value));     \
    __ movss(IceType_f64, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));   \
    __ movd(IceType_i32, XmmRegister::Encoded_Reg_##Dst,                       \
            GPRRegister::Encoded_Reg_##Src);                                   \
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

#define TestMovdXmmAddr(Dst, Value)                                            \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint32_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = 0xFFFFFFFF00000000ull;                                 \
                                                                               \
    __ movss(IceType_f64, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
    __ movd(IceType_i32, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));    \
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

#define TestMovd(Dst)                                                          \
  do {                                                                         \
    for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {   \
      TestMovdXmmReg(eax, Dst, Value);                                         \
      TestMovdXmmReg(ebx, Dst, Value);                                         \
      TestMovdXmmReg(ecx, Dst, Value);                                         \
      TestMovdXmmReg(edx, Dst, Value);                                         \
      TestMovdXmmReg(esi, Dst, Value);                                         \
      TestMovdXmmReg(edi, Dst, Value);                                         \
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

#undef TestMovdXmmAddr
#undef TestMovdXmmReg
#undef TestMovd
}

TEST_F(AssemblerX8632Test, MovdFromXmm) {
#define TestMovdRegXmm(Src, Dst, Value)                                        \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value;                                                 \
                                                                               \
    __ movss(IceType_f64, XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));   \
    __ movd(IceType_i32, GPRRegister::Encoded_Reg_##Dst,                       \
            XmmRegister::Encoded_Reg_##Src);                                   \
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

#define TestMovdAddrXmm(Src, Value)                                            \
  do {                                                                         \
    assert(((Value)&0xFFFFFFFF) == (Value));                                   \
    static constexpr char TestString[] = "(" #Src ", Addr)";                   \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value;                                                 \
    const uint32_t T1 = allocateDword();                                       \
    const uint32_t V1 = ~(Value);                                              \
                                                                               \
    __ movss(IceType_f64, XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));   \
    __ movd(IceType_i32, dwordAddress(T1), XmmRegister::Encoded_Reg_##Src);    \
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

#define TestMovd(Src)                                                          \
  do {                                                                         \
    for (uint32_t Value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu}) {   \
      TestMovdRegXmm(Src, eax, Value);                                         \
      TestMovdRegXmm(Src, ebx, Value);                                         \
      TestMovdRegXmm(Src, ecx, Value);                                         \
      TestMovdRegXmm(Src, edx, Value);                                         \
      TestMovdRegXmm(Src, esi, Value);                                         \
      TestMovdRegXmm(Src, edi, Value);                                         \
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

#undef TestMovdAddrXmm
#undef TestMovdRegXmm
#undef TestMovd
}

TEST_F(AssemblerX8632Test, MovqXmmAddr) {
#define TestMovd(Dst, Value)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movss(IceType_f64, XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));   \
    __ movq(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));                 \
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
  }

#undef TestMovd
}

TEST_F(AssemblerX8632Test, MovqAddrXmm) {
#define TestMovd(Dst, Value)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", Addr)";                   \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movq(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));                 \
    __ movq(dwordAddress(T1), XmmRegister::Encoded_Reg_##Dst);                 \
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
  }

#undef TestMovd
}

TEST_F(AssemblerX8632Test, MovqXmmXmm) {
#define TestMovd(Src, Dst, Value)                                              \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ", " #Dst ")";               \
    const uint32_t T0 = allocateQword();                                       \
    const uint64_t V0 = Value;                                                 \
    const uint32_t T1 = allocateQword();                                       \
    const uint64_t V1 = ~(Value);                                              \
                                                                               \
    __ movq(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));                 \
    __ movq(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));                 \
    __ movq(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src);   \
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
    TestMovd(xmm7, xmm0, Value);
  }

#undef TestMovd
}

TEST_F(AssemblerX8632Test, MovupsXmmAddr) {
#define TestMovups(Dst)                                                        \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ")";                         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
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

#undef TestMovups
}

TEST_F(AssemblerX8632Test, MovupsAddrXmm) {
#define TestMovups(Src)                                                        \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Src ")";                         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ movups(dwordAddress(T1), XmmRegister::Encoded_Reg_##Src);               \
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

#undef TestMovups
}

TEST_F(AssemblerX8632Test, MovupsXmmXmm) {
#define TestMovups(Dst, Src)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ")";               \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src); \
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
  TestMovups(xmm7, xmm0);

#undef TestMovups
}

TEST_F(AssemblerX8632Test, MovapsXmmXmm) {
#define TestMovaps(Dst, Src)                                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ")";               \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0(1.0f, -1.0, std::numeric_limits<float>::quiet_NaN(),       \
                    std::numeric_limits<float>::infinity());                   \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(0.0, 0.0, 0.0, 0.0);                                       \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));               \
    __ movaps(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src); \
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
  TestMovaps(xmm7, xmm0);

#undef TestMovaps
}

TEST_F(AssemblerX8632Test, Movhlps_Movlhps) {
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
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T1));               \
    __ Inst(XmmRegister::Encoded_Reg_##Dst, XmmRegister::Encoded_Reg_##Src);   \
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
  TestImpl(xmm7, xmm0);

#undef TestImpl
#undef TestImplSingle
}

TEST_F(AssemblerX8632Test, Movmsk) {
#define TestMovmskGPRXmm(GPR, Src, Value1, Expected, Inst)                     \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #GPR ", " #Src ", " #Value1 ", " #Expected ", " #Inst ")";         \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value1;                                                    \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ Inst(IceType_v4f32, GPRRegister::Encoded_Reg_##GPR,                     \
            XmmRegister::Encoded_Reg_##Src);                                   \
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

  TestMovmsk(eax, xmm0);
  TestMovmsk(ebx, xmm1);
  TestMovmsk(ecx, xmm2);
  TestMovmsk(edx, xmm3);
  TestMovmsk(esi, xmm4);
  TestMovmsk(edi, xmm5);
  TestMovmsk(eax, xmm6);
  TestMovmsk(ebx, xmm7);

#undef TestMovmskGPRXmm
#undef TestMovmsk
}

TEST_F(AssemblerX8632Test, Pmovsxdq) {
#define TestPmovsxdqXmmXmm(Dst, Src, Value1)                                   \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Dst ", " #Src ", " #Value1 ")";  \
    const uint32_t T0 = allocateDqword();                                      \
    const Dqword V0 Value1;                                                    \
    const uint32_t T1 = allocateDqword();                                      \
    const Dqword V1(uint64_t(0), uint64_t(0));                                 \
                                                                               \
    __ movups(XmmRegister::Encoded_Reg_##Src, dwordAddress(T0));               \
    __ movups(XmmRegister::Encoded_Reg_##Dst, dwordAddress(T1));               \
    __ pmovsxdq(XmmRegister::Encoded_Reg_##Dst,                                \
                XmmRegister::Encoded_Reg_##Src);                               \
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
  TestPmovsxdq(xmm7, xmm0);

#undef TestPmovsxdq
#undef TestPmovsxdqXmmXmm
}

TEST_F(AssemblerX8632Test, CmovRegReg) {
#define TestCmovRegReg(C, Src0, Value0, Src1, Value1, Dest, IsTrue)            \
  do {                                                                         \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src0, Immediate(Value0));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src1, Immediate(Value1));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dest, Immediate(Value0));   \
    __ cmp(IceType_i32, GPRRegister::Encoded_Reg_##Src0,                       \
           GPRRegister::Encoded_Reg_##Src1);                                   \
    __ cmov(IceType_i32, Cond::Br_##C, GPRRegister::Encoded_Reg_##Dest,        \
            GPRRegister::Encoded_Reg_##Src1);                                  \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
    ASSERT_EQ((IsTrue) ? (Value1) : (Value0), test.Dest())                     \
        << "(" #C ", " #Src0 ", " #Value0 ", " #Src1 ", " #Value1 ", " #Dest   \
           ", " #IsTrue ")";                                                   \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestCmovRegReg(o, eax, 0x80000000u, ebx, 0x1u, ecx, 1u);
  TestCmovRegReg(o, eax, 0x1u, ebx, 0x10000000u, ecx, 0u);

  TestCmovRegReg(no, ebx, 0x1u, ecx, 0x10000000u, edx, 1u);
  TestCmovRegReg(no, ebx, 0x80000000u, ecx, 0x1u, edx, 0u);

  TestCmovRegReg(b, ecx, 0x1, edx, 0x80000000u, eax, 1u);
  TestCmovRegReg(b, ecx, 0x80000000u, edx, 0x1u, eax, 0u);

  TestCmovRegReg(ae, edx, 0x80000000u, edi, 0x1u, ebx, 1u);
  TestCmovRegReg(ae, edx, 0x1u, edi, 0x80000000u, ebx, 0u);

  TestCmovRegReg(e, edi, 0x1u, esi, 0x1u, ecx, 1u);
  TestCmovRegReg(e, edi, 0x1u, esi, 0x11111u, ecx, 0u);

  TestCmovRegReg(ne, esi, 0x80000000u, eax, 0x1u, edx, 1u);
  TestCmovRegReg(ne, esi, 0x1u, eax, 0x1u, edx, 0u);

  TestCmovRegReg(be, eax, 0x1u, ebx, 0x80000000u, eax, 1u);
  TestCmovRegReg(be, eax, 0x80000000u, ebx, 0x1u, eax, 0u);

  TestCmovRegReg(a, ebx, 0x80000000u, ecx, 0x1u, ebx, 1u);
  TestCmovRegReg(a, ebx, 0x1u, ecx, 0x80000000u, ebx, 0u);

  TestCmovRegReg(s, ecx, 0x1u, edx, 0x80000000u, ecx, 1u);
  TestCmovRegReg(s, ecx, 0x80000000u, edx, 0x1u, ecx, 0u);

  TestCmovRegReg(ns, edx, 0x80000000u, edi, 0x1u, ecx, 1u);
  TestCmovRegReg(ns, edx, 0x1u, edi, 0x80000000u, ecx, 0u);

  TestCmovRegReg(p, edi, 0x80000000u, esi, 0x1u, edx, 1u);
  TestCmovRegReg(p, edi, 0x1u, esi, 0x80000000u, edx, 0u);

  TestCmovRegReg(np, esi, 0x1u, edi, 0x80000000u, eax, 1u);
  TestCmovRegReg(np, esi, 0x80000000u, edi, 0x1u, eax, 0u);

  TestCmovRegReg(l, edi, 0x80000000u, eax, 0x1u, ebx, 1u);
  TestCmovRegReg(l, edi, 0x1u, eax, 0x80000000u, ebx, 0u);

  TestCmovRegReg(ge, eax, 0x1u, ebx, 0x80000000u, ecx, 1u);
  TestCmovRegReg(ge, eax, 0x80000000u, ebx, 0x1u, ecx, 0u);

  TestCmovRegReg(le, ebx, 0x80000000u, ecx, 0x1u, edx, 1u);
  TestCmovRegReg(le, ebx, 0x1u, ecx, 0x80000000u, edx, 0u);

#undef TestCmovRegReg
}

TEST_F(AssemblerX8632Test, CmovRegAddr) {
#define TestCmovRegAddr(C, Src0, Value0, Value1, Dest, IsTrue)                 \
  do {                                                                         \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = Value1;                                                \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Src0, Immediate(Value0));   \
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_##Dest, Immediate(Value0));   \
    __ cmp(IceType_i32, GPRRegister::Encoded_Reg_##Src0, dwordAddress(T0));    \
    __ cmov(IceType_i32, Cond::Br_##C, GPRRegister::Encoded_Reg_##Dest,        \
            dwordAddress(T0));                                                 \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
    ASSERT_EQ((IsTrue) ? (Value1) : (Value0), test.Dest())                     \
        << "(" #C ", " #Src0 ", " #Value0 ", " #Value1 ", " #Dest ", " #IsTrue \
           ")";                                                                \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestCmovRegAddr(o, eax, 0x80000000u, 0x1u, ecx, 1u);
  TestCmovRegAddr(o, eax, 0x1u, 0x10000000u, ecx, 0u);

  TestCmovRegAddr(no, ebx, 0x1u, 0x10000000u, edx, 1u);
  TestCmovRegAddr(no, ebx, 0x80000000u, 0x1u, edx, 0u);

  TestCmovRegAddr(b, ecx, 0x1, 0x80000000u, eax, 1u);
  TestCmovRegAddr(b, ecx, 0x80000000u, 0x1u, eax, 0u);

  TestCmovRegAddr(ae, edx, 0x80000000u, 0x1u, ebx, 1u);
  TestCmovRegAddr(ae, edx, 0x1u, 0x80000000u, ebx, 0u);

  TestCmovRegAddr(e, edi, 0x1u, 0x1u, ecx, 1u);
  TestCmovRegAddr(e, edi, 0x1u, 0x11111u, ecx, 0u);

  TestCmovRegAddr(ne, esi, 0x80000000u, 0x1u, edx, 1u);
  TestCmovRegAddr(ne, esi, 0x1u, 0x1u, edx, 0u);

  TestCmovRegAddr(be, eax, 0x1u, 0x80000000u, eax, 1u);
  TestCmovRegAddr(be, eax, 0x80000000u, 0x1u, eax, 0u);

  TestCmovRegAddr(a, ebx, 0x80000000u, 0x1u, ebx, 1u);
  TestCmovRegAddr(a, ebx, 0x1u, 0x80000000u, ebx, 0u);

  TestCmovRegAddr(s, ecx, 0x1u, 0x80000000u, ecx, 1u);
  TestCmovRegAddr(s, ecx, 0x80000000u, 0x1u, ecx, 0u);

  TestCmovRegAddr(ns, edx, 0x80000000u, 0x1u, ecx, 1u);
  TestCmovRegAddr(ns, edx, 0x1u, 0x80000000u, ecx, 0u);

  TestCmovRegAddr(p, edi, 0x80000000u, 0x1u, edx, 1u);
  TestCmovRegAddr(p, edi, 0x1u, 0x80000000u, edx, 0u);

  TestCmovRegAddr(np, esi, 0x1u, 0x80000000u, eax, 1u);
  TestCmovRegAddr(np, esi, 0x80000000u, 0x1u, eax, 0u);

  TestCmovRegAddr(l, edi, 0x80000000u, 0x1u, ebx, 1u);
  TestCmovRegAddr(l, edi, 0x1u, 0x80000000u, ebx, 0u);

  TestCmovRegAddr(ge, eax, 0x1u, 0x80000000u, ecx, 1u);
  TestCmovRegAddr(ge, eax, 0x80000000u, 0x1u, ecx, 0u);

  TestCmovRegAddr(le, ebx, 0x80000000u, 0x1u, edx, 1u);
  TestCmovRegAddr(le, ebx, 0x1u, 0x80000000u, edx, 0u);

#undef TestCmovRegAddr
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice
