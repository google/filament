//===- subzero/unittest/AssemblerX8632/X87.cpp ----------------------------===//
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

TEST_F(AssemblerX8632LowLevelTest, Fld) {
  __ fld(IceType_f32,
         Address(GPRRegister::Encoded_Reg_ebp, 1, AssemblerFixup::NoFixup));
  __ fld(IceType_f64, Address(GPRRegister::Encoded_Reg_ebp, 0x10000,
                              AssemblerFixup::NoFixup));

  constexpr size_t ByteCount = 9;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Fld32Opcode = 0xd9;
  constexpr uint8_t Fld32ModRM = (/*mod*/ 1 << 6) | (/*reg*/ 0 << 3) |
                                 (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  constexpr uint8_t Fld64Opcode = 0xdd;
  constexpr uint8_t Fld64ModRM = (/*mod*/ 2 << 6) | (/*reg*/ 0 << 3) |
                                 (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  verifyBytes<ByteCount>(codeBytes(), Fld32Opcode, Fld32ModRM, 0x01,
                         Fld64Opcode, Fld64ModRM, 0x00, 0x00, 0x01, 0x00);
}

TEST_F(AssemblerX8632LowLevelTest, FstpAddr) {
  __ fstp(IceType_f32,
          Address(GPRRegister::Encoded_Reg_ebp, 1, AssemblerFixup::NoFixup));
  __ fstp(IceType_f64, Address(GPRRegister::Encoded_Reg_ebp, 0x10000,
                               AssemblerFixup::NoFixup));

  constexpr size_t ByteCount = 9;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Fld32Opcode = 0xd9;
  constexpr uint8_t Fld32ModRM = (/*mod*/ 1 << 6) | (/*reg*/ 3 << 3) |
                                 (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  constexpr uint8_t Fld64Opcode = 0xdd;
  constexpr uint8_t Fld64ModRM = (/*mod*/ 2 << 6) | (/*reg*/ 3 << 3) |
                                 (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  verifyBytes<ByteCount>(codeBytes(), Fld32Opcode, Fld32ModRM, 0x01,
                         Fld64Opcode, Fld64ModRM, 0x00, 0x00, 0x01, 0x00);
}

TEST_F(AssemblerX8632LowLevelTest, Fincstp) {
  __ fincstp();

  constexpr size_t ByteCount = 2;
  ASSERT_EQ(ByteCount, codeBytesSize());

  verifyBytes<ByteCount>(codeBytes(), 0xD9, 0XF7);
}

TEST_F(AssemblerX8632LowLevelTest, FnstcwAddr) {
  __ fnstcw(
      Address(GPRRegister::Encoded_Reg_ebp, 0x12345, AssemblerFixup::NoFixup));

  constexpr size_t ByteCount = 6;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Opcode = 0xd9;
  constexpr uint8_t ModRM = (/*mod*/ 2 << 6) | (/*reg*/ 7 << 3) |
                            (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  verifyBytes<ByteCount>(codeBytes(), Opcode, ModRM, 0x45, 0x23, 0x01, 0x00);
}

TEST_F(AssemblerX8632LowLevelTest, FldcwAddr) {
  __ fldcw(
      Address(GPRRegister::Encoded_Reg_ebp, 0x12345, AssemblerFixup::NoFixup));

  constexpr size_t ByteCount = 6;
  ASSERT_EQ(ByteCount, codeBytesSize());

  constexpr uint8_t Opcode = 0xd9;
  constexpr uint8_t ModRM = (/*mod*/ 2 << 6) | (/*reg*/ 5 << 3) |
                            (/*rm*/ GPRRegister::Encoded_Reg_ebp);
  verifyBytes<ByteCount>(codeBytes(), Opcode, ModRM, 0x45, 0x23, 0x01, 0x00);
}

TEST_F(AssemblerX8632Test, FstpSt) {
#define TestFstpSt(Size, MemorySize, Type)                                     \
  do {                                                                         \
    const uint32_t T1 = allocate##MemorySize();                                \
    const Type OldValue1 = -1.0f;                                              \
    const uint32_t T2 = allocate##MemorySize();                                \
    const Type OldValue2 = -2.0f;                                              \
    const uint32_t T3 = allocate##MemorySize();                                \
    const Type OldValue3 = -3.0f;                                              \
    const uint32_t T4 = allocate##MemorySize();                                \
    const Type OldValue4 = -4.0f;                                              \
    const uint32_t T5 = allocate##MemorySize();                                \
    const Type OldValue5 = -5.0f;                                              \
    const uint32_t T6 = allocate##MemorySize();                                \
    const Type OldValue6 = -6.0f;                                              \
    const uint32_t T7 = allocate##MemorySize();                                \
    const Type OldValue7 = -7.0f;                                              \
                                                                               \
    const uint32_t N7 = allocate##MemorySize();                                \
    constexpr Type NewValue7 = 777.77f;                                        \
    const uint32_t N6 = allocate##MemorySize();                                \
    constexpr Type NewValue6 = 666.66f;                                        \
    const uint32_t N5 = allocate##MemorySize();                                \
    constexpr Type NewValue5 = 555.55f;                                        \
    const uint32_t N4 = allocate##MemorySize();                                \
    constexpr Type NewValue4 = 444.44f;                                        \
    const uint32_t N3 = allocate##MemorySize();                                \
    constexpr Type NewValue3 = 333.33f;                                        \
    const uint32_t N2 = allocate##MemorySize();                                \
    constexpr Type NewValue2 = 222.22f;                                        \
    const uint32_t N1 = allocate##MemorySize();                                \
    constexpr Type NewValue1 = 111.11f;                                        \
                                                                               \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
    __ fincstp();                                                              \
                                                                               \
    __ fld(IceType_f##Size, dwordAddress(N7));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_7);                                   \
    __ fld(IceType_f##Size, dwordAddress(N6));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_6);                                   \
    __ fld(IceType_f##Size, dwordAddress(N5));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_5);                                   \
    __ fld(IceType_f##Size, dwordAddress(N4));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_4);                                   \
    __ fld(IceType_f##Size, dwordAddress(N3));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_3);                                   \
    __ fld(IceType_f##Size, dwordAddress(N2));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_2);                                   \
    __ fld(IceType_f##Size, dwordAddress(N1));                                 \
    __ fstp(X87STRegister::Encoded_X87ST_1);                                   \
                                                                               \
    __ fstp(IceType_f##Size, dwordAddress(T1));                                \
    __ fstp(IceType_f##Size, dwordAddress(T2));                                \
    __ fstp(IceType_f##Size, dwordAddress(T3));                                \
    __ fstp(IceType_f##Size, dwordAddress(T4));                                \
    __ fstp(IceType_f##Size, dwordAddress(T5));                                \
    __ fstp(IceType_f##Size, dwordAddress(T6));                                \
    __ fstp(IceType_f##Size, dwordAddress(T7));                                \
                                                                               \
    AssembledTest test = assemble();                                           \
    test.set##MemorySize##To(T1, OldValue1);                                   \
    test.set##MemorySize##To(N1, NewValue1);                                   \
    test.set##MemorySize##To(T2, OldValue2);                                   \
    test.set##MemorySize##To(N2, NewValue2);                                   \
    test.set##MemorySize##To(T3, OldValue3);                                   \
    test.set##MemorySize##To(N3, NewValue3);                                   \
    test.set##MemorySize##To(T4, OldValue4);                                   \
    test.set##MemorySize##To(N4, NewValue4);                                   \
    test.set##MemorySize##To(T5, OldValue5);                                   \
    test.set##MemorySize##To(N5, NewValue5);                                   \
    test.set##MemorySize##To(T6, OldValue6);                                   \
    test.set##MemorySize##To(N6, NewValue6);                                   \
    test.set##MemorySize##To(T7, OldValue7);                                   \
    test.set##MemorySize##To(N7, NewValue7);                                   \
                                                                               \
    test.run();                                                                \
                                                                               \
    ASSERT_FLOAT_EQ(NewValue1, test.contentsOf##MemorySize<Type>(T1))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue1, test.contentsOf##MemorySize<Type>(N1))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue2, test.contentsOf##MemorySize<Type>(T2))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue2, test.contentsOf##MemorySize<Type>(N2))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue3, test.contentsOf##MemorySize<Type>(T3))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue3, test.contentsOf##MemorySize<Type>(N3))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue4, test.contentsOf##MemorySize<Type>(T4))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue4, test.contentsOf##MemorySize<Type>(N4))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue5, test.contentsOf##MemorySize<Type>(T5))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue5, test.contentsOf##MemorySize<Type>(N5))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue6, test.contentsOf##MemorySize<Type>(T6))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue6, test.contentsOf##MemorySize<Type>(N6))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue7, test.contentsOf##MemorySize<Type>(T7))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
    ASSERT_FLOAT_EQ(NewValue7, test.contentsOf##MemorySize<Type>(N7))          \
        << "(" #Size ", " #MemorySize ", " #Type ")";                          \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestFstpSt(32, Dword, float);
  TestFstpSt(64, Qword, double);

#undef TestFstpSt
}

TEST_F(AssemblerX8632Test, Fild) {
#define TestFild(OperandType, Size, MemorySize, FpType, IntType)               \
  do {                                                                         \
    const uint32_t T0 = allocate##MemorySize();                                \
    constexpr IntType V0 = 0x1234;                                             \
                                                                               \
    __ fild##OperandType(dwordAddress(T0));                                    \
    __ fstp(IceType_f##Size, dwordAddress(T0));                                \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.set##MemorySize##To(T0, V0);                                          \
    test.run();                                                                \
                                                                               \
    ASSERT_FLOAT_EQ(static_cast<FpType>(V0),                                   \
                    test.contentsOf##MemorySize<FpType>(T0))                   \
        << "(" #OperandType ", " #Size ", " #MemorySize ", " #FpType           \
           ", " #IntType ")";                                                  \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestFild(s, 32, Dword, float, uint32_t);
  TestFild(l, 64, Qword, double, uint64_t);
#undef TestFild
}

TEST_F(AssemblerX8632Test, Fistp) {
#define TestFistp(OperandType, Size, MemorySize, FpType, IntType)              \
  do {                                                                         \
    const uint32_t T0 = allocate##MemorySize();                                \
    constexpr IntType V0 = 0x1234;                                             \
    const uint32_t T1 = allocate##MemorySize();                                \
    constexpr IntType V1 = 0xFFFF;                                             \
                                                                               \
    __ fild##OperandType(dwordAddress(T0));                                    \
    __ fistp##OperandType(dwordAddress(T1));                                   \
                                                                               \
    AssembledTest test = assemble();                                           \
                                                                               \
    test.set##MemorySize##To(T0, V0);                                          \
    test.set##MemorySize##To(T1, V1);                                          \
    test.run();                                                                \
                                                                               \
    ASSERT_EQ(static_cast<IntType>(V0),                                        \
              test.contentsOf##MemorySize<IntType>(T0))                        \
        << "(" #OperandType ", " #Size ", " #MemorySize ", " #FpType           \
           ", " #IntType ")";                                                  \
    ASSERT_EQ(static_cast<IntType>(V0),                                        \
              test.contentsOf##MemorySize<IntType>(T1))                        \
        << "(" #OperandType ", " #Size ", " #MemorySize ", " #FpType           \
           ", " #IntType ")";                                                  \
                                                                               \
    reset();                                                                   \
  } while (0)

  TestFistp(s, 32, Dword, float, uint32_t);
  TestFistp(l, 64, Qword, double, uint64_t);
#undef TestFistp
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice
