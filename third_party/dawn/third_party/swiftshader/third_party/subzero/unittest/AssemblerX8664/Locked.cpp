//===- subzero/unittest/AssemblerX8664/Locked.cpp -------------------------===//
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

TEST_F(AssemblerX8664LowLevelTest, Mfence) {
  __ mfence();

  static constexpr uint8_t ByteCount = 3;
  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), 0x0F, 0xAE, 0xF0);
}

TEST_F(AssemblerX8664LowLevelTest, Lock) {
  __ lock();

  static constexpr uint8_t ByteCount = 1;
  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), 0xF0);
}

TEST_F(AssemblerX8664Test, Xchg) {
  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplAddrReg(Value0, Dst1, Value1, Size)                            \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Value0 ", " #Dst1 ", " #Value1 ", " #Size ")";                    \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = (Value0)&Mask##Size;                                   \
    const uint32_t V1 = (Value1)&Mask##Size;                                   \
                                                                               \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst1(), Immediate(Value1));          \
    __ xchg(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Dst1());          \
    __ And(IceType_i32, Encoded_GPR_##Dst1(), Immediate(Mask##Size));          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Dst1()) << TestString;                                  \
    ASSERT_EQ(V1, test.contentsOfDword(T0)) << TestString;                     \
    reset();                                                                   \
  } while (0)

#define TestImplSize(Dst1, Size)                                               \
  do {                                                                         \
    TestImplAddrReg(0xa2b34567, Dst1, 0x0507ddee, Size);                       \
  } while (0)

#define TestImpl(Dst1)                                                         \
  do {                                                                         \
    TestImplSize(Dst1, 8);                                                     \
    TestImplSize(Dst1, 16);                                                    \
    TestImplSize(Dst1, 32);                                                    \
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
#undef TestImplSize
#undef TestImplAddrReg

#define TestImplRegReg(Reg0, Value0, Reg1, Value1, Size)                       \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Reg0 "," #Value0 ", " #Reg1 ", " #Value1 ", " #Size ")";          \
    const uint32_t V0 = (Value0)&Mask##Size;                                   \
    const uint32_t V1 = (Value1)&Mask##Size;                                   \
                                                                               \
    __ mov(IceType_i##Size, Encoded_GPR_##Reg0(), Immediate(Value0));          \
    __ mov(IceType_i##Size, Encoded_GPR_##Reg1(), Immediate(Value1));          \
    __ xchg(IceType_i##Size, Encoded_GPR_##Reg0(), Encoded_GPR_##Reg1());      \
    __ And(IceType_i32, Encoded_GPR_##Reg0(), Immediate(Mask##Size));          \
    __ And(IceType_i32, Encoded_GPR_##Reg1(), Immediate(Mask##Size));          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Reg1()) << TestString;                                  \
    ASSERT_EQ(V1, test.Reg0()) << TestString;                                  \
    reset();                                                                   \
  } while (0)

#define TestImplSize(Reg0, Reg1, Size)                                         \
  do {                                                                         \
    TestImplRegReg(Reg0, 0xa2b34567, Reg1, 0x0507ddee, Size);                  \
  } while (0)

#define TestImpl(Reg0, Reg1)                                                   \
  do {                                                                         \
    TestImplSize(Reg0, Reg1, 8);                                               \
    TestImplSize(Reg0, Reg1, 16);                                              \
    TestImplSize(Reg0, Reg1, 32);                                              \
  } while (0)

  // r1 == rax so has a short encoding
  TestImpl(r6, r1);
  TestImpl(r1, r8);

  TestImpl(r2, r10);
  TestImpl(r3, r11);
  TestImpl(r4, r12);
  TestImpl(r5, r13);
  TestImpl(r6, r14);
  TestImpl(r7, r15);

#undef TestImpl
#undef TestImplSize
#undef TestImplRegReg
}

TEST_F(AssemblerX8664Test, Xadd) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplAddrReg(Value0, Dst1, Value1, LockedOrNot, Size)               \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Value0 ", " #Dst1 ", " #Value1 ", " #Size ")";                    \
    const uint32_t T0 = allocateDword();                                       \
    const uint32_t V0 = (Value0)&Mask##Size;                                   \
    const uint32_t V1 = (Value1)&Mask##Size;                                   \
                                                                               \
    __ mov(IceType_i##Size, Encoded_GPR_##Dst1(), Immediate(Value1));          \
    __ xadd(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Dst1(),           \
            LockedOrNot);                                                      \
    __ And(IceType_i32, Encoded_GPR_##Dst1(), Immediate(Mask##Size));          \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(V0, test.Dst1()) << TestString;                                  \
    ASSERT_EQ(Mask##Size &(V1 + V0), test.contentsOfDword(T0)) << TestString;  \
    reset();                                                                   \
  } while (0)

#define TestImplSize(Dst1, Size)                                               \
  do {                                                                         \
    TestImplAddrReg(0xa2b34567, Dst1, 0x0507ddee, NotLocked, Size);            \
    TestImplAddrReg(0xa2b34567, Dst1, 0x0507ddee, Locked, Size);               \
  } while (0)

#define TestImpl(Dst1)                                                         \
  do {                                                                         \
    TestImplSize(Dst1, 8);                                                     \
    TestImplSize(Dst1, 16);                                                    \
    TestImplSize(Dst1, 32);                                                    \
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
#undef TestImplSize
#undef TestImplAddrReg
}

TEST_F(AssemblerX8664LowLevelTest, Xadd) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

  // Ensures that xadd emits a lock prefix accordingly.
  {
    __ xadd(IceType_i8, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
            NotLocked);
    static constexpr uint8_t ByteCountNotLocked8 = 10;
    ASSERT_EQ(ByteCountNotLocked8, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked8>(codeBytes(), 0x67, 0x44, 0x0F,
                                                 0xC0, 0x34, 0x25, 0x00, 0xFF,
                                                 0x01, 0x00));
    reset();

    __ xadd(IceType_i8, Address::Absolute(0x1FF00), Encoded_GPR_r14(), Locked);
    static constexpr uint8_t ByteCountLocked8 = 1 + ByteCountNotLocked8;
    ASSERT_EQ(ByteCountLocked8, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked8>(codeBytes(), 0xF0, 0x67, 0x44,
                                              0x0F, 0xC0, 0x34, 0x25, 0x00,
                                              0xFF, 0x01, 0x00));
    reset();
  }

  {
    __ xadd(IceType_i16, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
            NotLocked);
    static constexpr uint8_t ByteCountNotLocked16 = 11;
    ASSERT_EQ(ByteCountNotLocked16, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked16>(codeBytes(), 0x66, 0x67, 0x44,
                                                  0x0F, 0xC1, 0x34, 0x25, 0x00,
                                                  0xFF, 0x01, 0x00));
    reset();

    __ xadd(IceType_i16, Address::Absolute(0x1FF00), Encoded_GPR_r14(), Locked);
    static constexpr uint8_t ByteCountLocked16 = 1 + ByteCountNotLocked16;
    ASSERT_EQ(ByteCountLocked16, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked16>(codeBytes(), 0x66, 0xF0, 0x67,
                                               0x44, 0x0F, 0xC1, 0x34, 0x25,
                                               0x00, 0xFF, 0x01, 0x00));
    reset();
  }

  {
    __ xadd(IceType_i32, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
            NotLocked);
    static constexpr uint8_t ByteCountNotLocked32 = 10;
    ASSERT_EQ(ByteCountNotLocked32, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked32>(codeBytes(), 0x67, 0x44, 0x0F,
                                                  0xC1, 0x34, 0x25, 0x00, 0xFF,
                                                  0x01, 0x00));
    reset();

    __ xadd(IceType_i32, Address::Absolute(0x1FF00), Encoded_GPR_r14(), Locked);
    static constexpr uint8_t ByteCountLocked32 = 1 + ByteCountNotLocked32;
    ASSERT_EQ(ByteCountLocked32, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked32>(codeBytes(), 0xF0, 0x67, 0x44,
                                               0x0F, 0xC1, 0x34, 0x25, 0x00,
                                               0xFF, 0x01, 0x00));
    reset();
  }
}

TEST_F(AssemblerX8664LowLevelTest, EmitSegmentOverride) {
#define TestImpl(Prefix)                                                       \
  do {                                                                         \
    static constexpr uint8_t ByteCount = 1;                                    \
    __ emitSegmentOverride(Prefix);                                            \
    ASSERT_EQ(ByteCount, codeBytesSize()) << Prefix;                           \
    ASSERT_TRUE(verifyBytes<ByteCount>(codeBytes(), Prefix));                  \
    reset();                                                                   \
  } while (0)

  TestImpl(0x26);
  TestImpl(0x2E);
  TestImpl(0x36);
  TestImpl(0x3E);
  TestImpl(0x64);
  TestImpl(0x65);
  TestImpl(0x66);
  TestImpl(0x67);

#undef TestImpl
}

TEST_F(AssemblerX8664Test, Cmpxchg8b) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

#define TestImpl(Value0, Value1, ValueMem, LockedOrNot)                        \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Value0 ", " #Value1 ", " #ValueMem ", " #LockedOrNot ")";         \
    const uint32_t T0 = allocateQword();                                       \
    static constexpr uint64_t V0 = ValueMem;                                   \
    const uint32_t ZeroFlag = allocateDword();                                 \
                                                                               \
    __ mov(IceType_i32, Encoded_GPR_eax(),                                     \
           Immediate(uint64_t(Value0) & 0xFFFFFFFF));                          \
    __ mov(IceType_i32, Encoded_GPR_edx(), Immediate(uint64_t(Value0) >> 32)); \
    __ mov(IceType_i32, Encoded_GPR_ebx(),                                     \
           Immediate(uint64_t(Value1) & 0xFFFFFFFF));                          \
    __ mov(IceType_i32, Encoded_GPR_ecx(), Immediate(uint64_t(Value1) >> 32)); \
    __ cmpxchg8b(dwordAddress(T0), LockedOrNot);                               \
    __ setcc(Cond::Br_e, dwordAddress(ZeroFlag));                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setQwordTo(T0, V0);                                                   \
    test.setDwordTo(ZeroFlag, uint32_t(0xFF));                                 \
    test.run();                                                                \
                                                                               \
    if (V0 == (Value0)) {                                                      \
      ASSERT_EQ(uint64_t(Value1), test.contentsOfQword(T0)) << TestString;     \
      ASSERT_EQ(1u, test.contentsOfDword(ZeroFlag)) << TestString;             \
    } else {                                                                   \
      ASSERT_EQ(uint64_t(ValueMem) & 0xFFFFFFFF, test.eax()) << TestString;    \
      ASSERT_EQ((uint64_t(ValueMem) >> 32) & 0xFFFFFFFF, test.edx())           \
          << TestString;                                                       \
      ASSERT_EQ(0u, test.contentsOfDword(ZeroFlag)) << TestString;             \
    }                                                                          \
    reset();                                                                   \
  } while (0)

  TestImpl(0x98987676543210ull, 0x1, 0x98987676543210ull, NotLocked);
  TestImpl(0x98987676543210ull, 0x1, 0x98987676543210ull, Locked);
  TestImpl(0x98987676543210ull, 0x1, 0x98987676543211ull, NotLocked);
  TestImpl(0x98987676543210ull, 0x1, 0x98987676543211ull, Locked);

#undef TestImpl
}

TEST_F(AssemblerX8664LowLevelTest, Cmpxchg8b) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

  // Ensures that cmpxchg8b emits a lock prefix accordingly.
  __ cmpxchg8b(Address::Absolute(0x1FF00), NotLocked);
  static constexpr uint8_t ByteCountNotLocked = 9;
  ASSERT_EQ(ByteCountNotLocked, codeBytesSize());
  ASSERT_TRUE(verifyBytes<ByteCountNotLocked>(
      codeBytes(), 0x67, 0x0F, 0xC7, 0x0C, 0x25, 0x00, 0xFF, 0x01, 0x00));
  reset();

  __ cmpxchg8b(Address::Absolute(0x1FF00), Locked);
  static constexpr uint8_t ByteCountLocked = 1 + ByteCountNotLocked;
  ASSERT_EQ(ByteCountLocked, codeBytesSize());
  ASSERT_TRUE(verifyBytes<ByteCountLocked>(codeBytes(), 0xF0, 0x67, 0x0F, 0xC7,
                                           0x0C, 0x25, 0x00, 0xFF, 0x01, 0x00));
  reset();
}

TEST_F(AssemblerX8664Test, Cmpxchg) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

  static constexpr uint32_t Mask8 = 0x000000FF;
  static constexpr uint32_t Mask16 = 0x0000FFFF;
  static constexpr uint32_t Mask32 = 0xFFFFFFFF;

#define TestImplAddrReg(Value0, Src, Value1, ValueMem, LockedOrNot, Size)      \
  do {                                                                         \
    static constexpr char TestString[] =                                       \
        "(" #Value0 ", " #Src ", " #Value1 ", " #ValueMem ", " #LockedOrNot    \
        ", " #Size ")";                                                        \
    const uint32_t T0 = allocateDword();                                       \
    static constexpr uint32_t V0 = (ValueMem)&Mask##Size;                      \
    const uint32_t ZeroFlag = allocateDword();                                 \
                                                                               \
    __ mov(IceType_i##Size, Encoded_GPR_eax(),                                 \
           Immediate((Value0)&Mask##Size));                                    \
    __ mov(IceType_i##Size, Encoded_GPR_##Src(),                               \
           Immediate((Value1)&Mask##Size));                                    \
    __ cmpxchg(IceType_i##Size, dwordAddress(T0), Encoded_GPR_##Src(),         \
               LockedOrNot);                                                   \
    __ setcc(Cond::Br_e, dwordAddress(ZeroFlag));                              \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.setDwordTo(T0, V0);                                                   \
    test.setDwordTo(ZeroFlag, uint32_t(0xFF));                                 \
    test.run();                                                                \
                                                                               \
    if (V0 == (Mask##Size & (Value0))) {                                       \
      ASSERT_EQ(uint32_t((Value1)&Mask##Size), test.contentsOfDword(T0))       \
          << TestString;                                                       \
      ASSERT_EQ(1u, test.contentsOfDword(ZeroFlag)) << TestString;             \
    } else {                                                                   \
      ASSERT_EQ(uint32_t((ValueMem)&Mask##Size), test.eax()) << TestString;    \
      ASSERT_EQ(0u, test.contentsOfDword(ZeroFlag)) << TestString;             \
    }                                                                          \
    reset();                                                                   \
  } while (0)

#define TestImplValue(Value0, Src, Value1, ValueMem, LockedOrNot)              \
  do {                                                                         \
    TestImplAddrReg(Value0, Src, Value1, ValueMem, LockedOrNot, 8);            \
    TestImplAddrReg(Value0, Src, Value1, ValueMem, LockedOrNot, 16);           \
    TestImplAddrReg(Value0, Src, Value1, ValueMem, LockedOrNot, 32);           \
  } while (0)

#define TestImpl(Src, LockedOrNot)                                             \
  do {                                                                         \
    TestImplValue(0xFFFFFFFF, Src, 0x1, 0xFFFFFFFF, LockedOrNot);              \
    TestImplValue(0x0FFF0F0F, Src, 0x1, 0xFFFFFFFF, LockedOrNot);              \
  } while (0)

  TestImpl(r2, Locked);
  TestImpl(r2, NotLocked);
  TestImpl(r3, Locked);
  TestImpl(r3, NotLocked);
  TestImpl(r4, Locked);
  TestImpl(r4, NotLocked);
  TestImpl(r5, Locked);
  TestImpl(r5, NotLocked);
  TestImpl(r6, Locked);
  TestImpl(r6, NotLocked);
  TestImpl(r7, Locked);
  TestImpl(r7, NotLocked);
  TestImpl(r8, Locked);
  TestImpl(r8, NotLocked);
  TestImpl(r10, Locked);
  TestImpl(r10, NotLocked);
  TestImpl(r11, Locked);
  TestImpl(r11, NotLocked);
  TestImpl(r12, Locked);
  TestImpl(r12, NotLocked);
  TestImpl(r13, Locked);
  TestImpl(r13, NotLocked);
  TestImpl(r14, Locked);
  TestImpl(r14, NotLocked);
  TestImpl(r15, Locked);
  TestImpl(r15, NotLocked);

#undef TestImpl
#undef TestImplValue
#undef TestImplAddrReg
}

TEST_F(AssemblerX8664LowLevelTest, Cmpxchg) {
  static constexpr bool NotLocked = false;
  static constexpr bool Locked = true;

  // Ensures that cmpxchg emits a lock prefix accordingly.
  {
    __ cmpxchg(IceType_i8, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               NotLocked);
    static constexpr uint8_t ByteCountNotLocked8 = 10;
    ASSERT_EQ(ByteCountNotLocked8, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked8>(codeBytes(), 0x67, 0x44, 0x0F,
                                                 0xB0, 0x34, 0x25, 0x00, 0xFF,
                                                 0x01, 0x00));
    reset();

    __ cmpxchg(IceType_i8, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               Locked);
    static constexpr uint8_t ByteCountLocked8 = 1 + ByteCountNotLocked8;
    ASSERT_EQ(ByteCountLocked8, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked8>(codeBytes(), 0xF0, 0x67, 0x44,
                                              0x0F, 0xB0, 0x34, 0x25, 0x00,
                                              0xFF, 0x01, 0x00));
    reset();
  }

  {
    __ cmpxchg(IceType_i16, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               NotLocked);
    static constexpr uint8_t ByteCountNotLocked16 = 11;
    ASSERT_EQ(ByteCountNotLocked16, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked16>(codeBytes(), 0x66, 0x67, 0x44,
                                                  0x0F, 0xB1, 0x34, 0x25, 0x00,
                                                  0xFF, 0x01, 0x00));
    reset();

    __ cmpxchg(IceType_i16, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               Locked);
    static constexpr uint8_t ByteCountLocked16 = 1 + ByteCountNotLocked16;
    ASSERT_EQ(ByteCountLocked16, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked16>(codeBytes(), 0x66, 0xF0, 0x67,
                                               0x44, 0x0F, 0xB1, 0x34, 0x25,
                                               0x00, 0xFF, 0x01, 0x00));
    reset();
  }

  {
    __ cmpxchg(IceType_i32, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               NotLocked);
    static constexpr uint8_t ByteCountNotLocked32 = 10;
    ASSERT_EQ(ByteCountNotLocked32, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountNotLocked32>(codeBytes(), 0x67, 0x44, 0x0F,
                                                  0xB1, 0x34, 0x25, 0x00, 0xFF,
                                                  0x01, 0x00));
    reset();

    __ cmpxchg(IceType_i32, Address::Absolute(0x1FF00), Encoded_GPR_r14(),
               Locked);
    static constexpr uint8_t ByteCountLocked32 = 1 + ByteCountNotLocked32;
    ASSERT_EQ(ByteCountLocked32, codeBytesSize());
    ASSERT_TRUE(verifyBytes<ByteCountLocked32>(codeBytes(), 0xF0, 0x67, 0x44,
                                               0x0F, 0xB1, 0x34, 0x25, 0x00,
                                               0xFF, 0x01, 0x00));
    reset();
  }
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
