
//===- subzero/unittest/unittest/AssemblerX8664/TestUtil.h ------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Utility classes for testing the X8664 Assembler.
//
//===----------------------------------------------------------------------===//

#ifndef ASSEMBLERX8664_TESTUTIL_H_
#define ASSEMBLERX8664_TESTUTIL_H_

#include "IceAssemblerX8664.h"

#include "gtest/gtest.h"

#if defined(__unix__)
#include <sys/mman.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#else
#error "Platform unsupported"
#endif

#include <cassert>

namespace Ice {
namespace X8664 {
namespace Test {

class AssemblerX8664TestBase : public ::testing::Test {
protected:
  using Address = AssemblerX8664::Traits::Address;
  using Cond = AssemblerX8664::CondX86;
  using GPRRegister = AssemblerX8664::Traits::GPRRegister;
  using ByteRegister = AssemblerX8664::Traits::ByteRegister;
  using Traits = AssemblerX8664::Traits;
  using XmmRegister = AssemblerX8664::Traits::XmmRegister;

// The following are "nicknames" for all possible GPRs in x86-64. With those, we
// can use, e.g.,
//
//  Encoded_GPR_al()
//
// instead of GPRRegister::Encoded_Reg_eax for 8 bit operands. They also
// introduce "regular" nicknames for legacy x86-32 register (e.g., eax becomes
// r1; esp, r0).
#define LegacyRegAliases(NewName, Name64, Name32, Name16, Name8)               \
  static constexpr GPRRegister Encoded_GPR_##NewName() {                       \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##NewName##q() {                    \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##NewName##d() {                    \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##NewName##w() {                    \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##NewName##l() {                    \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr ByteRegister Encoded_Bytereg_##NewName() {                  \
    return ByteRegister::Encoded_8_Reg_##Name8;                                \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name64() {                        \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name32() {                        \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name16() {                        \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name8() {                         \
    return GPRRegister::Encoded_Reg_##Name32;                                  \
  }
#define NewRegAliases(Name)                                                    \
  static constexpr GPRRegister Encoded_GPR_##Name() {                          \
    return GPRRegister::Encoded_Reg_##Name##d;                                 \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name##q() {                       \
    return GPRRegister::Encoded_Reg_##Name##d;                                 \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name##d() {                       \
    return GPRRegister::Encoded_Reg_##Name##d;                                 \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name##w() {                       \
    return GPRRegister::Encoded_Reg_##Name##d;                                 \
  }                                                                            \
  static constexpr GPRRegister Encoded_GPR_##Name##l() {                       \
    return GPRRegister::Encoded_Reg_##Name##d;                                 \
  }                                                                            \
  static constexpr ByteRegister Encoded_Bytereg_##Name() {                     \
    return ByteRegister::Encoded_8_Reg_##Name##l;                              \
  }
#define XmmRegAliases(Name)                                                    \
  static constexpr XmmRegister Encoded_Xmm_##Name() {                          \
    return XmmRegister::Encoded_Reg_##Name;                                    \
  }
  LegacyRegAliases(r0, rsp, esp, sp, spl);
  LegacyRegAliases(r1, rax, eax, ax, al);
  LegacyRegAliases(r2, rbx, ebx, bx, bl);
  LegacyRegAliases(r3, rcx, ecx, cx, cl);
  LegacyRegAliases(r4, rdx, edx, dx, dl);
  LegacyRegAliases(r5, rbp, ebp, bp, bpl);
  LegacyRegAliases(r6, rsi, esi, si, sil);
  LegacyRegAliases(r7, rdi, edi, di, dil);
  NewRegAliases(r8);
  NewRegAliases(r9);
  NewRegAliases(r10);
  NewRegAliases(r11);
  NewRegAliases(r12);
  NewRegAliases(r13);
  NewRegAliases(r14);
  NewRegAliases(r15);
  XmmRegAliases(xmm0);
  XmmRegAliases(xmm1);
  XmmRegAliases(xmm2);
  XmmRegAliases(xmm3);
  XmmRegAliases(xmm4);
  XmmRegAliases(xmm5);
  XmmRegAliases(xmm6);
  XmmRegAliases(xmm7);
  XmmRegAliases(xmm8);
  XmmRegAliases(xmm9);
  XmmRegAliases(xmm10);
  XmmRegAliases(xmm11);
  XmmRegAliases(xmm12);
  XmmRegAliases(xmm13);
  XmmRegAliases(xmm14);
  XmmRegAliases(xmm15);
#undef XmmRegAliases
#undef NewRegAliases
#undef LegacyRegAliases

  AssemblerX8664TestBase() { reset(); }

  void reset() { Assembler = makeUnique<AssemblerX8664>(); }

  AssemblerX8664 *assembler() const { return Assembler.get(); }

  size_t codeBytesSize() const { return Assembler->getBufferView().size(); }

  const uint8_t *codeBytes() const {
    return static_cast<const uint8_t *>(
        static_cast<const void *>(Assembler->getBufferView().data()));
  }

private:
  std::unique_ptr<AssemblerX8664> Assembler;
};

// __ is a helper macro. It allows test cases to emit X8664 assembly
// instructions with
//
//   __ mov(GPRRegister::Reg_Eax, 1);
//   __ ret();
//
// and so on. The idea of having this was "stolen" from dart's unit tests.
#define __ (this->assembler())->

// AssemblerX8664LowLevelTest verify that the "basic" instructions the tests
// rely on are encoded correctly. Therefore, instead of executing the assembled
// code, these tests will verify that the assembled bytes are sane.
class AssemblerX8664LowLevelTest : public AssemblerX8664TestBase {
protected:
  // verifyBytes is a template helper that takes a Buffer, and a variable number
  // of bytes. As the name indicates, it is used to verify the bytes for an
  // instruction encoding.
  template <int N, int I> static bool verifyBytes(const uint8_t *) {
    static_assert(I == N, "Invalid template instantiation.");
    return true;
  }

  template <int N, int I = 0, typename... Args>
  static bool verifyBytes(const uint8_t *Buffer, uint8_t Byte,
                          Args... OtherBytes) {
    static_assert(I < N, "Invalid template instantiation.");
    EXPECT_EQ(Byte, Buffer[I]) << "Byte " << (I + 1) << " of " << N;
    return verifyBytes<N, I + 1>(Buffer, OtherBytes...) && Buffer[I] == Byte;
  }
};

// After these tests we should have a sane environment; we know the following
// work:
//
//  (*) zeroing eax, ebx, ecx, edx, edi, and esi;
//  (*) call $4 instruction (used for ip materialization);
//  (*) register push and pop;
//  (*) cmp reg, reg; and
//  (*) returning from functions.
//
// We can now dive into testing each emitting method in AssemblerX8664. Each
// test will emit some instructions for performing the test. The assembled
// instructions will operate in a "safe" environment. All x86-64 registers are
// spilled to the program stack, and the registers are then zeroed out, with the
// exception of %esp and %r9.
//
// The jitted code and the unittest code will share the same stack. Therefore,
// test harnesses need to ensure it does not leave anything it pushed on the
// stack.
//
// %r9 is initialized with a pointer for rIP-based addressing. This pointer is
// used for position-independent access to a scratchpad area for use in tests.
// In theory we could use rip-based addressing, but in practice that would
// require creating fixups, which would, in turn, require creating a global
// context. We therefore rely on the same technique used for pic code in x86-32
// (i.e., IP materialization). Upon a test start up, a call(NextInstruction) is
// executed. We then pop the return address from the stack, and use it for pic
// addressing.
//
// The jitted code will look like the following:
//
// test:
//       push   %r9
//       call   test$materialize_ip
// test$materialize_ip:                           <<------- %r9 will point here
//       pop    %r9
//       push   %rax
//       push   %rbx
//       push   %rcx
//       push   %rdx
//       push   %rbp
//       push   %rdi
//       push   %rsi
//       push   %r8
//       push   %r10
//       push   %r11
//       push   %r12
//       push   %r13
//       push   %r14
//       push   %r15
//       mov    $0, %rax
//       mov    $0, %rbx
//       mov    $0, %rcx
//       mov    $0, %rdx
//       mov    $0, %rbp
//       mov    $0, %rdi
//       mov    $0, %rsi
//       mov    $0, %r8
//       mov    $0, %r10
//       mov    $0, %r11
//       mov    $0, %r12
//       mov    $0, %r13
//       mov    $0, %r14
//       mov    $0, %r15
//
//       << test code goes here >>
//
//       mov    %rax, {  0 + $ScratchpadOffset}(%rbp)
//       mov    %rbx, {  8 + $ScratchpadOffset}(%rbp)
//       mov    %rcx, { 16 + $ScratchpadOffset}(%rbp)
//       mov    %rdx, { 24 + $ScratchpadOffset}(%rbp)
//       mov    %rdi, { 32 + $ScratchpadOffset}(%rbp)
//       mov    %rsi, { 40 + $ScratchpadOffset}(%rbp)
//       mov    %rbp, { 48 + $ScratchpadOffset}(%rbp)
//       mov    %rsp, { 56 + $ScratchpadOffset}(%rbp)
//       mov    %r8,  { 64 + $ScratchpadOffset}(%rbp)
//       mov    %r9,  { 72 + $ScratchpadOffset}(%rbp)
//       mov    %r10, { 80 + $ScratchpadOffset}(%rbp)
//       mov    %r11, { 88 + $ScratchpadOffset}(%rbp)
//       mov    %r12, { 96 + $ScratchpadOffset}(%rbp)
//       mov    %r13, {104 + $ScratchpadOffset}(%rbp)
//       mov    %r14, {112 + $ScratchpadOffset}(%rbp)
//       mov    %r15, {120 + $ScratchpadOffset}(%rbp)
//       movups %xmm0,  {128 + $ScratchpadOffset}(%rbp)
//       movups %xmm1,  {136 + $ScratchpadOffset}(%rbp)
//       movups %xmm2,  {144 + $ScratchpadOffset}(%rbp)
//       movups %xmm3,  {152 + $ScratchpadOffset}(%rbp)
//       movups %xmm4,  {160 + $ScratchpadOffset}(%rbp)
//       movups %xmm5,  {168 + $ScratchpadOffset}(%rbp)
//       movups %xmm6,  {176 + $ScratchpadOffset}(%rbp)
//       movups %xmm7,  {184 + $ScratchpadOffset}(%rbp)
//       movups %xmm8,  {192 + $ScratchpadOffset}(%rbp)
//       movups %xmm9,  {200 + $ScratchpadOffset}(%rbp)
//       movups %xmm10, {208 + $ScratchpadOffset}(%rbp)
//       movups %xmm11, {216 + $ScratchpadOffset}(%rbp)
//       movups %xmm12, {224 + $ScratchpadOffset}(%rbp)
//       movups %xmm13, {232 + $ScratchpadOffset}(%rbp)
//       movups %xmm14, {240 + $ScratchpadOffset}(%rbp)
//       movups %xmm15, {248 + $ScratchpadOffset}(%rbp)
//
//       pop    %r15
//       pop    %r14
//       pop    %r13
//       pop    %r12
//       pop    %r11
//       pop    %r10
//       pop    %r8
//       pop    %rsi
//       pop    %rdi
//       pop    %rbp
//       pop    %rdx
//       pop    %rcx
//       pop    %rbx
//       pop    %rax
//       pop    %r9
//       ret
//
//      << ... >>
//
// scratchpad:                              <<------- accessed via $Offset(%ebp)
//
//      << test scratch area >>
//
// TODO(jpp): test the
//
//    mov %reg, $Offset(%ebp)
//    movups %xmm, $Offset(%ebp)
//
// encodings using the low level assembler test ensuring that the register
// values can be written to the scratchpad area.
//
// r9 was deliberately choosen so that every instruction accessing memory would
// fail if the rex prefix was not emitted for it.
class AssemblerX8664Test : public AssemblerX8664TestBase {
protected:
  // Dqword is used to represent 128-bit data types. The Dqword's contents are
  // the same as the contents read from memory. Tests can then use the union
  // members to verify the tests' outputs.
  //
  // NOTE: We want sizeof(Dqword) == sizeof(uint64_t) * 2. In other words, we
  // want Dqword's contents to be **exactly** what the memory contents were so
  // that we can do, e.g.,
  //
  // ...
  // float Ret[4];
  // // populate Ret
  // return *reinterpret_cast<Dqword *>(&Ret);
  //
  // While being an ugly hack, this kind of return statements are used
  // extensively in the PackedArith (see below) class.
  union Dqword {
    template <typename T0, typename T1, typename T2, typename T3,
              typename = typename std::enable_if<
                  std::is_floating_point<T0>::value>::type>
    Dqword(T0 F0, T1 F1, T2 F2, T3 F3) {
      F32[0] = F0;
      F32[1] = F1;
      F32[2] = F2;
      F32[3] = F3;
    }

    template <typename T>
    Dqword(typename std::enable_if<std::is_same<T, int32_t>::value, T>::type I0,
           T I1, T I2, T I3) {
      I32[0] = I0;
      I32[1] = I1;
      I32[2] = I2;
      I32[3] = I3;
    }

    template <typename T>
    Dqword(typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type
               U64_0,
           T U64_1) {
      U64[0] = U64_0;
      U64[1] = U64_1;
    }

    template <typename T>
    Dqword(typename std::enable_if<std::is_same<T, double>::value, T>::type D0,
           T D1) {
      F64[0] = D0;
      F64[1] = D1;
    }

    bool operator==(const Dqword &Rhs) const {
      return std::memcmp(this, &Rhs, sizeof(*this)) == 0;
    }

    double F64[2];
    uint64_t U64[2];
    int64_t I64[2];

    float F32[4];
    uint32_t U32[4];
    int32_t I32[4];

    uint16_t U16[8];
    int16_t I16[8];

    uint8_t U8[16];
    int8_t I8[16];

  private:
    Dqword() = delete;
  };

  // As stated, we want this condition to hold, so we assert.
  static_assert(sizeof(Dqword) == 2 * sizeof(uint64_t),
                "Dqword has the wrong size.");

  // PackedArith is an interface provider for Dqwords. PackedArith's C argument
  // is the undelying Dqword's type, which is then used so that we can define
  // operators in terms of C++ operators on the underlying elements' type.
  template <typename C> class PackedArith {
  public:
    static constexpr uint32_t N = sizeof(Dqword) / sizeof(C);
    static_assert(N * sizeof(C) == sizeof(Dqword),
                  "Invalid template paramenter.");
    static_assert((N & 1) == 0, "N should be divisible by 2");

#define DefinePackedComparisonOperator(Op)                                     \
  template <typename Container = C, int Size = N>                              \
  typename std::enable_if<std::is_floating_point<Container>::value,            \
                          Dqword>::type                                        \
  operator Op(const Dqword &Rhs) const {                                       \
    using ElemType =                                                           \
        typename std::conditional<std::is_same<float, Container>::value,       \
                                  int32_t, int64_t>::type;                     \
    static_assert(sizeof(ElemType) == sizeof(Container),                       \
                  "Check ElemType definition.");                               \
    const ElemType *const RhsPtr =                                             \
        reinterpret_cast<const ElemType *const>(&Rhs);                         \
    const ElemType *const LhsPtr =                                             \
        reinterpret_cast<const ElemType *const>(&Lhs);                         \
    ElemType Ret[N];                                                           \
    for (uint32_t i = 0; i < N; ++i) {                                         \
      Ret[i] = (LhsPtr[i] Op RhsPtr[i]) ? -1 : 0;                              \
    }                                                                          \
    return *reinterpret_cast<Dqword *>(&Ret);                                  \
  }

    DefinePackedComparisonOperator(<);
    DefinePackedComparisonOperator(<=);
    DefinePackedComparisonOperator(>);
    DefinePackedComparisonOperator(>=);
    DefinePackedComparisonOperator(==);
    DefinePackedComparisonOperator(!=);

#undef DefinePackedComparisonOperator

#define DefinePackedOrdUnordComparisonOperator(Op, Ordered)                    \
  template <typename Container = C, int Size = N>                              \
  typename std::enable_if<std::is_floating_point<Container>::value,            \
                          Dqword>::type                                        \
  Op(const Dqword &Rhs) const {                                                \
    using ElemType =                                                           \
        typename std::conditional<std::is_same<float, Container>::value,       \
                                  int32_t, int64_t>::type;                     \
    static_assert(sizeof(ElemType) == sizeof(Container),                       \
                  "Check ElemType definition.");                               \
    const Container *const RhsPtr =                                            \
        reinterpret_cast<const Container *const>(&Rhs);                        \
    const Container *const LhsPtr =                                            \
        reinterpret_cast<const Container *const>(&Lhs);                        \
    ElemType Ret[N];                                                           \
    for (uint32_t i = 0; i < N; ++i) {                                         \
      Ret[i] = (!(LhsPtr[i] == LhsPtr[i]) || !(RhsPtr[i] == RhsPtr[i])) !=     \
                       (Ordered)                                               \
                   ? -1                                                        \
                   : 0;                                                        \
    }                                                                          \
    return *reinterpret_cast<Dqword *>(&Ret);                                  \
  }

    DefinePackedOrdUnordComparisonOperator(ord, true);
    DefinePackedOrdUnordComparisonOperator(unord, false);
#undef DefinePackedOrdUnordComparisonOperator

#define DefinePackedArithOperator(Op, RhsIndexChanges, NeedsInt)               \
  template <typename Container = C, int Size = N>                              \
  Dqword operator Op(const Dqword &Rhs) const {                                \
    using ElemTypeForFp = typename std::conditional<                           \
        !(NeedsInt), Container,                                                \
        typename std::conditional<                                             \
            std::is_same<Container, float>::value, uint32_t,                   \
            typename std::conditional<std::is_same<Container, double>::value,  \
                                      uint64_t, void>::type>::type>::type;     \
    using ElemType =                                                           \
        typename std::conditional<std::is_integral<Container>::value,          \
                                  Container, ElemTypeForFp>::type;             \
    static_assert(!std::is_same<void, ElemType>::value,                        \
                  "Check ElemType definition.");                               \
    const ElemType *const RhsPtr =                                             \
        reinterpret_cast<const ElemType *const>(&Rhs);                         \
    const ElemType *const LhsPtr =                                             \
        reinterpret_cast<const ElemType *const>(&Lhs);                         \
    ElemType Ret[N];                                                           \
    for (uint32_t i = 0; i < N; ++i) {                                         \
      Ret[i] = LhsPtr[i] Op RhsPtr[(RhsIndexChanges) ? i : 0];                 \
    }                                                                          \
    return *reinterpret_cast<Dqword *>(&Ret);                                  \
  }

    DefinePackedArithOperator(>>, false, true);
    DefinePackedArithOperator(<<, false, true);
    DefinePackedArithOperator(+, true, false);
    DefinePackedArithOperator(-, true, false);
    DefinePackedArithOperator(/, true, false);
    DefinePackedArithOperator(&, true, true);
    DefinePackedArithOperator(|, true, true);
    DefinePackedArithOperator(^, true, true);

#undef DefinePackedArithOperator

#define DefinePackedArithShiftImm(Op)                                          \
  template <typename Container = C, int Size = N>                              \
  Dqword operator Op(uint8_t imm) const {                                      \
    const Container *const LhsPtr =                                            \
        reinterpret_cast<const Container *const>(&Lhs);                        \
    Container Ret[N];                                                          \
    for (uint32_t i = 0; i < N; ++i) {                                         \
      Ret[i] = LhsPtr[i] Op imm;                                               \
    }                                                                          \
    return *reinterpret_cast<Dqword *>(&Ret);                                  \
  }

    DefinePackedArithShiftImm(>>);
    DefinePackedArithShiftImm(<<);

#undef DefinePackedArithShiftImm

    template <typename Container = C, int Size = N>
    typename std::enable_if<std::is_signed<Container>::value ||
                                std::is_floating_point<Container>::value,
                            Dqword>::type
    operator*(const Dqword &Rhs) const {
      static_assert((std::is_integral<Container>::value &&
                     sizeof(Container) < sizeof(uint64_t)) ||
                        std::is_floating_point<Container>::value,
                    "* is only defined for i(8|16|32), and fp types.");

      const Container *const RhsPtr =
          reinterpret_cast<const Container *const>(&Rhs);
      const Container *const LhsPtr =
          reinterpret_cast<const Container *const>(&Lhs);
      Container Ret[Size];
      for (uint32_t i = 0; i < Size; ++i) {
        Ret[i] = LhsPtr[i] * RhsPtr[i];
      }
      return *reinterpret_cast<Dqword *>(&Ret);
    }

    template <typename Container = C, int Size = N,
              typename = typename std::enable_if<
                  !std::is_signed<Container>::value>::type>
    Dqword operator*(const Dqword &Rhs) const {
      static_assert(std::is_integral<Container>::value &&
                        sizeof(Container) < sizeof(uint64_t),
                    "* is only defined for ui(8|16|32)");
      using NextType = typename std::conditional<
          sizeof(Container) == 1, uint16_t,
          typename std::conditional<sizeof(Container) == 2, uint32_t,
                                    uint64_t>::type>::type;
      static_assert(sizeof(Container) * 2 == sizeof(NextType),
                    "Unexpected size");

      const Container *const RhsPtr =
          reinterpret_cast<const Container *const>(&Rhs);
      const Container *const LhsPtr =
          reinterpret_cast<const Container *const>(&Lhs);
      NextType Ret[Size / 2];
      for (uint32_t i = 0; i < Size; i += 2) {
        Ret[i / 2] =
            static_cast<NextType>(LhsPtr[i]) * static_cast<NextType>(RhsPtr[i]);
      }
      return *reinterpret_cast<Dqword *>(&Ret);
    }

    template <typename Container = C, int Size = N>
    PackedArith<Container> operator~() const {
      const Container *const LhsPtr =
          reinterpret_cast<const Container *const>(&Lhs);
      Container Ret[Size];
      for (uint32_t i = 0; i < Size; ++i) {
        Ret[i] = ~LhsPtr[i];
      }
      return PackedArith<Container>(*reinterpret_cast<Dqword *>(&Ret));
    }

#define MinMaxOperations(Name, Suffix)                                         \
  template <typename Container = C, int Size = N>                              \
  Dqword Name##Suffix(const Dqword &Rhs) const {                               \
    static_assert(std::is_floating_point<Container>::value,                    \
                  #Name #Suffix "ps is only available for fp.");               \
    const Container *const RhsPtr =                                            \
        reinterpret_cast<const Container *const>(&Rhs);                        \
    const Container *const LhsPtr =                                            \
        reinterpret_cast<const Container *const>(&Lhs);                        \
    Container Ret[Size];                                                       \
    for (uint32_t i = 0; i < Size; ++i) {                                      \
      Ret[i] = std::Name(LhsPtr[i], RhsPtr[i]);                                \
    }                                                                          \
    return *reinterpret_cast<Dqword *>(&Ret);                                  \
  }

    MinMaxOperations(max, ps);
    MinMaxOperations(max, pd);
    MinMaxOperations(min, ps);
    MinMaxOperations(min, pd);
#undef MinMaxOperations

    template <typename Container = C, int Size = N>
    Dqword blendWith(const Dqword &Rhs, const Dqword &Mask) const {
      using MaskType = typename std::conditional<
          sizeof(Container) == 1, int8_t,
          typename std::conditional<sizeof(Container) == 2, int16_t,
                                    int32_t>::type>::type;
      static_assert(sizeof(MaskType) == sizeof(Container),
                    "MaskType has the wrong size.");
      const Container *const RhsPtr =
          reinterpret_cast<const Container *const>(&Rhs);
      const Container *const LhsPtr =
          reinterpret_cast<const Container *const>(&Lhs);
      const MaskType *const MaskPtr =
          reinterpret_cast<const MaskType *const>(&Mask);
      Container Ret[Size];
      for (int i = 0; i < Size; ++i) {
        Ret[i] = ((MaskPtr[i] < 0) ? RhsPtr : LhsPtr)[i];
      }
      return *reinterpret_cast<Dqword *>(&Ret);
    }

  private:
    // The AssemblerX8664Test class needs to be a friend so that it can create
    // PackedArith objects (see below.)
    friend class AssemblerX8664Test;

    explicit PackedArith(const Dqword &MyLhs) : Lhs(MyLhs) {}

    // Lhs can't be a & because operator~ returns a temporary object that needs
    // access to its own Dqword.
    const Dqword Lhs;
  };

  // Named constructor for PackedArith objects.
  template <typename C> static PackedArith<C> packedAs(const Dqword &D) {
    return PackedArith<C>(D);
  }

  AssemblerX8664Test() { reset(); }

  void reset() {
    AssemblerX8664TestBase::reset();

    NeedsEpilogue = true;
    // These dwords are allocated for saving the GPR state after the jitted code
    // runs.
    NumAllocatedDwords = AssembledTest::ScratchpadSlots;
    addPrologue();
  }

  // AssembledTest is a wrapper around a PROT_EXEC mmap'ed buffer. This buffer
  // contains both the test code as well as prologue/epilogue, and the
  // scratchpad area that tests may use -- all tests use this scratchpad area
  // for storing the processor's registers after the tests executed. This class
  // also exposes helper methods for reading the register state after test
  // execution, as well as for reading the scratchpad area.
  class AssembledTest {
    AssembledTest() = delete;
    AssembledTest(const AssembledTest &) = delete;
    AssembledTest &operator=(const AssembledTest &) = delete;

  public:
    static constexpr uint32_t MaximumCodeSize = 1 << 20;
    static constexpr uint32_t raxSlot() { return 0; }
    static constexpr uint32_t rbxSlot() { return 2; }
    static constexpr uint32_t rcxSlot() { return 4; }
    static constexpr uint32_t rdxSlot() { return 6; }
    static constexpr uint32_t rdiSlot() { return 8; }
    static constexpr uint32_t rsiSlot() { return 10; }
    static constexpr uint32_t rbpSlot() { return 12; }
    static constexpr uint32_t rspSlot() { return 14; }
    static constexpr uint32_t r8Slot() { return 16; }
    static constexpr uint32_t r9Slot() { return 18; }
    static constexpr uint32_t r10Slot() { return 20; }
    static constexpr uint32_t r11Slot() { return 22; }
    static constexpr uint32_t r12Slot() { return 24; }
    static constexpr uint32_t r13Slot() { return 26; }
    static constexpr uint32_t r14Slot() { return 28; }
    static constexpr uint32_t r15Slot() { return 30; }

    // save 4 dwords for each xmm registers.
    static constexpr uint32_t xmm0Slot() { return 32; }
    static constexpr uint32_t xmm1Slot() { return 36; }
    static constexpr uint32_t xmm2Slot() { return 40; }
    static constexpr uint32_t xmm3Slot() { return 44; }
    static constexpr uint32_t xmm4Slot() { return 48; }
    static constexpr uint32_t xmm5Slot() { return 52; }
    static constexpr uint32_t xmm6Slot() { return 56; }
    static constexpr uint32_t xmm7Slot() { return 60; }
    static constexpr uint32_t xmm8Slot() { return 64; }
    static constexpr uint32_t xmm9Slot() { return 68; }
    static constexpr uint32_t xmm10Slot() { return 72; }
    static constexpr uint32_t xmm11Slot() { return 76; }
    static constexpr uint32_t xmm12Slot() { return 80; }
    static constexpr uint32_t xmm13Slot() { return 84; }
    static constexpr uint32_t xmm14Slot() { return 88; }
    static constexpr uint32_t xmm15Slot() { return 92; }

    static constexpr uint32_t ScratchpadSlots = 96;

    AssembledTest(const uint8_t *Data, const size_t MySize,
                  const size_t ExtraStorageDwords)
        : Size(MaximumCodeSize + 4 * ExtraStorageDwords) {
      // MaxCodeSize is needed because EXPECT_LT needs a symbol with a name --
      // probably a compiler bug?
      uint32_t MaxCodeSize = MaximumCodeSize;
      EXPECT_LT(MySize, MaxCodeSize);
      assert(MySize < MaximumCodeSize);

#if defined(__unix__)
      ExecutableData = mmap(nullptr, Size, PROT_WRITE | PROT_READ | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
      EXPECT_NE(MAP_FAILED, ExecutableData) << strerror(errno);
      assert(MAP_FAILED != ExecutableData);
#elif defined(_WIN32)
      ExecutableData = VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE,
                                    PAGE_EXECUTE_READWRITE);
      EXPECT_NE(nullptr, ExecutableData) << strerror(errno);
      assert(nullptr != ExecutableData);
#else
#error "Platform unsupported"
#endif

      std::memcpy(ExecutableData, Data, MySize);
    }

    // We allow AssembledTest to be moved so that we can return objects of
    // this type.
    AssembledTest(AssembledTest &&Buffer)
        : ExecutableData(Buffer.ExecutableData), Size(Buffer.Size) {
      Buffer.ExecutableData = nullptr;
      Buffer.Size = 0;
    }

    AssembledTest &operator=(AssembledTest &&Buffer) {
      ExecutableData = Buffer.ExecutableData;
      Buffer.ExecutableData = nullptr;
      Size = Buffer.Size;
      Buffer.Size = 0;
      return *this;
    }

    ~AssembledTest() {
      if (ExecutableData != nullptr) {
#if defined(__unix__)
        munmap(ExecutableData, Size);
#elif defined(_WIN32)
        VirtualFree(ExecutableData, 0, MEM_RELEASE);
#else
#error "Platform unsupported"
#endif
        ExecutableData = nullptr;
      }
    }

    void run() const { reinterpret_cast<void (*)()>(ExecutableData)(); }

#define LegacyRegAccessors(NewName, Name64, Name32, Name16, Name8)             \
  static_assert(Encoded_GPR_##NewName() == Encoded_GPR_##Name64(),             \
                "Invalid aliasing.");                                          \
  uint64_t NewName() const {                                                   \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##NewName##q() == Encoded_GPR_##Name64(),          \
                "Invalid aliasing.");                                          \
  uint64_t NewName##q() const {                                                \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##NewName##d() == Encoded_GPR_##Name64(),          \
                "Invalid aliasing.");                                          \
  uint32_t NewName##d() const {                                                \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##NewName##w() == Encoded_GPR_##Name64(),          \
                "Invalid aliasing.");                                          \
  uint16_t NewName##w() const {                                                \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##NewName##l() == Encoded_GPR_##Name64(),          \
                "Invalid aliasing.");                                          \
  uint8_t NewName##l() const {                                                 \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##Name64() == Encoded_GPR_##Name64(),              \
                "Invalid aliasing.");                                          \
  uint64_t Name64() const {                                                    \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##Name32() == Encoded_GPR_##Name64(),              \
                "Invalid aliasing.");                                          \
  uint32_t Name32() const {                                                    \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##Name16() == Encoded_GPR_##Name64(),              \
                "Invalid aliasing.");                                          \
  uint16_t Name16() const {                                                    \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }                                                                            \
  static_assert(Encoded_GPR_##Name8() == Encoded_GPR_##Name64(),               \
                "Invalid aliasing.");                                          \
  uint8_t Name8() const {                                                      \
    return contentsOfQword(AssembledTest::Name64##Slot());                     \
  }
#define NewRegAccessors(NewName)                                               \
  uint64_t NewName() const {                                                   \
    return contentsOfQword(AssembledTest::NewName##Slot());                    \
  }                                                                            \
  uint64_t NewName##q() const {                                                \
    return contentsOfQword(AssembledTest::NewName##Slot());                    \
  }                                                                            \
  uint32_t NewName##d() const {                                                \
    return contentsOfQword(AssembledTest::NewName##Slot());                    \
  }                                                                            \
  uint16_t NewName##w() const {                                                \
    return contentsOfQword(AssembledTest::NewName##Slot());                    \
  }                                                                            \
  uint8_t NewName##l() const {                                                 \
    return contentsOfQword(AssembledTest::NewName##Slot());                    \
  }
#define XmmRegAccessor(Name)                                                   \
  template <typename T> T Name() const {                                       \
    return xmm<T>(AssembledTest::Name##Slot());                                \
  }
    LegacyRegAccessors(r0, rsp, esp, sp, spl);
    LegacyRegAccessors(r1, rax, eax, ax, al);
    LegacyRegAccessors(r2, rbx, ebx, bx, bl);
    LegacyRegAccessors(r3, rcx, ecx, cx, cl);
    LegacyRegAccessors(r4, rdx, edx, dx, dl);
    LegacyRegAccessors(r5, rbp, ebp, bp, bpl);
    LegacyRegAccessors(r6, rsi, esi, si, sil);
    LegacyRegAccessors(r7, rdi, edi, di, dil);
    NewRegAccessors(r8);
    NewRegAccessors(r9);
    NewRegAccessors(r10);
    NewRegAccessors(r11);
    NewRegAccessors(r12);
    NewRegAccessors(r13);
    NewRegAccessors(r14);
    NewRegAccessors(r15);
    XmmRegAccessor(xmm0);
    XmmRegAccessor(xmm1);
    XmmRegAccessor(xmm2);
    XmmRegAccessor(xmm3);
    XmmRegAccessor(xmm4);
    XmmRegAccessor(xmm5);
    XmmRegAccessor(xmm6);
    XmmRegAccessor(xmm7);
    XmmRegAccessor(xmm8);
    XmmRegAccessor(xmm9);
    XmmRegAccessor(xmm10);
    XmmRegAccessor(xmm11);
    XmmRegAccessor(xmm12);
    XmmRegAccessor(xmm13);
    XmmRegAccessor(xmm14);
    XmmRegAccessor(xmm15);
#undef XmmRegAccessor
#undef NewRegAccessors
#undef LegacyRegAccessors

    // contentsOfDword is used for reading the values in the scratchpad area.
    // Valid arguments are the dword ids returned by
    // AssemblerX8664Test::allocateDword() -- other inputs are considered
    // invalid, and are not guaranteed to work if the implementation changes.
    template <typename T = uint32_t, typename = typename std::enable_if<
                                         sizeof(T) == sizeof(uint32_t)>::type>
    T contentsOfDword(uint32_t Dword) const {
      return *reinterpret_cast<T *>(static_cast<uint8_t *>(ExecutableData) +
                                    dwordOffset(Dword));
    }

    template <typename T = uint64_t, typename = typename std::enable_if<
                                         sizeof(T) == sizeof(uint64_t)>::type>
    T contentsOfQword(uint32_t InitialDword) const {
      return *reinterpret_cast<T *>(static_cast<uint8_t *>(ExecutableData) +
                                    dwordOffset(InitialDword));
    }

    Dqword contentsOfDqword(uint32_t InitialDword) const {
      return *reinterpret_cast<Dqword *>(
          static_cast<uint8_t *>(ExecutableData) + dwordOffset(InitialDword));
    }

    template <typename T = uint32_t, typename = typename std::enable_if<
                                         sizeof(T) == sizeof(uint32_t)>::type>
    void setDwordTo(uint32_t Dword, T value) {
      *reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(ExecutableData) +
                                    dwordOffset(Dword)) =
          *reinterpret_cast<uint32_t *>(&value);
    }

    template <typename T = uint64_t, typename = typename std::enable_if<
                                         sizeof(T) == sizeof(uint64_t)>::type>
    void setQwordTo(uint32_t InitialDword, T value) {
      *reinterpret_cast<uint64_t *>(static_cast<uint8_t *>(ExecutableData) +
                                    dwordOffset(InitialDword)) =
          *reinterpret_cast<uint64_t *>(&value);
    }

    void setDqwordTo(uint32_t InitialDword, const Dqword &qdword) {
      setQwordTo(InitialDword, qdword.U64[0]);
      setQwordTo(InitialDword + 2, qdword.U64[1]);
    }

  private:
    template <typename T>
    typename std::enable_if<std::is_same<T, Dqword>::value, Dqword>::type
    xmm(uint8_t Slot) const {
      return contentsOfDqword(Slot);
    }

    template <typename T>
    typename std::enable_if<!std::is_same<T, Dqword>::value, T>::type
    xmm(uint8_t Slot) const {
      constexpr bool TIs64Bit = sizeof(T) == sizeof(uint64_t);
      using _64BitType = typename std::conditional<TIs64Bit, T, uint64_t>::type;
      using _32BitType = typename std::conditional<TIs64Bit, uint32_t, T>::type;
      if (TIs64Bit) {
        return contentsOfQword<_64BitType>(Slot);
      }
      return contentsOfDword<_32BitType>(Slot);
    }

    static uint32_t dwordOffset(uint32_t Index) {
      return MaximumCodeSize + (Index * 4);
    }

    void *ExecutableData = nullptr;
    size_t Size;
  };

  // assemble created an AssembledTest with the jitted code. The first time
  // assemble is executed it will add the epilogue to the jitted code (which is
  // the reason why this method is not const qualified.
  AssembledTest assemble() {
    if (NeedsEpilogue) {
      addEpilogue();
    }
    NeedsEpilogue = false;

    for (const auto *Fixup : assembler()->fixups()) {
      Fixup->emitOffset(assembler());
    }

    return AssembledTest(codeBytes(), codeBytesSize(), NumAllocatedDwords);
  }

  // Allocates a new dword slot in the test's scratchpad area.
  uint32_t allocateDword() { return NumAllocatedDwords++; }

  // Allocates a new qword slot in the test's scratchpad area.
  uint32_t allocateQword() {
    uint32_t InitialDword = allocateDword();
    allocateDword();
    return InitialDword;
  }

  // Allocates a new dqword slot in the test's scratchpad area.
  uint32_t allocateDqword() {
    uint32_t InitialDword = allocateQword();
    allocateQword();
    return InitialDword;
  }

  Address dwordAddress(uint32_t Dword) {
    return Address(Encoded_GPR_r9(), dwordDisp(Dword), nullptr);
  }

private:
  // e??SlotAddress returns an AssemblerX8664::Traits::Address that can be used
  // by the test cases to encode an address operand for accessing the slot for
  // the specified register. These are all private for, when jitting the test
  // code, tests should not tamper with these values. Besides, during the test
  // execution these slots' contents are undefined and should not be accessed.
  Address raxSlotAddress() { return dwordAddress(AssembledTest::raxSlot()); }
  Address rbxSlotAddress() { return dwordAddress(AssembledTest::rbxSlot()); }
  Address rcxSlotAddress() { return dwordAddress(AssembledTest::rcxSlot()); }
  Address rdxSlotAddress() { return dwordAddress(AssembledTest::rdxSlot()); }
  Address rdiSlotAddress() { return dwordAddress(AssembledTest::rdiSlot()); }
  Address rsiSlotAddress() { return dwordAddress(AssembledTest::rsiSlot()); }
  Address rbpSlotAddress() { return dwordAddress(AssembledTest::rbpSlot()); }
  Address rspSlotAddress() { return dwordAddress(AssembledTest::rspSlot()); }
  Address r8SlotAddress() { return dwordAddress(AssembledTest::r8Slot()); }
  Address r9SlotAddress() { return dwordAddress(AssembledTest::r9Slot()); }
  Address r10SlotAddress() { return dwordAddress(AssembledTest::r10Slot()); }
  Address r11SlotAddress() { return dwordAddress(AssembledTest::r11Slot()); }
  Address r12SlotAddress() { return dwordAddress(AssembledTest::r12Slot()); }
  Address r13SlotAddress() { return dwordAddress(AssembledTest::r13Slot()); }
  Address r14SlotAddress() { return dwordAddress(AssembledTest::r14Slot()); }
  Address r15SlotAddress() { return dwordAddress(AssembledTest::r15Slot()); }
  Address xmm0SlotAddress() { return dwordAddress(AssembledTest::xmm0Slot()); }
  Address xmm1SlotAddress() { return dwordAddress(AssembledTest::xmm1Slot()); }
  Address xmm2SlotAddress() { return dwordAddress(AssembledTest::xmm2Slot()); }
  Address xmm3SlotAddress() { return dwordAddress(AssembledTest::xmm3Slot()); }
  Address xmm4SlotAddress() { return dwordAddress(AssembledTest::xmm4Slot()); }
  Address xmm5SlotAddress() { return dwordAddress(AssembledTest::xmm5Slot()); }
  Address xmm6SlotAddress() { return dwordAddress(AssembledTest::xmm6Slot()); }
  Address xmm7SlotAddress() { return dwordAddress(AssembledTest::xmm7Slot()); }
  Address xmm8SlotAddress() { return dwordAddress(AssembledTest::xmm8Slot()); }
  Address xmm9SlotAddress() { return dwordAddress(AssembledTest::xmm9Slot()); }
  Address xmm10SlotAddress() {
    return dwordAddress(AssembledTest::xmm10Slot());
  }
  Address xmm11SlotAddress() {
    return dwordAddress(AssembledTest::xmm11Slot());
  }
  Address xmm12SlotAddress() {
    return dwordAddress(AssembledTest::xmm12Slot());
  }
  Address xmm13SlotAddress() {
    return dwordAddress(AssembledTest::xmm13Slot());
  }
  Address xmm14SlotAddress() {
    return dwordAddress(AssembledTest::xmm14Slot());
  }
  Address xmm15SlotAddress() {
    return dwordAddress(AssembledTest::xmm15Slot());
  }

  // Returns the displacement that should be used when accessing the specified
  // Dword in the scratchpad area. It needs to adjust for the initial
  // instructions that are emitted before the call that materializes the IP
  // register.
  uint32_t dwordDisp(uint32_t Dword) const {
    EXPECT_LT(Dword, NumAllocatedDwords);
    assert(Dword < NumAllocatedDwords);
    static constexpr uint8_t PushR9Bytes = 2;
    static constexpr uint8_t CallImmBytes = 5;
    return AssembledTest::MaximumCodeSize + (Dword * 4) -
           (PushR9Bytes + CallImmBytes);
  }

  void addPrologue() {
    __ pushl(Encoded_GPR_r9());
    __ call(Immediate(4));
    __ popl(Encoded_GPR_r9());

    __ pushl(Encoded_GPR_rax());
    __ pushl(Encoded_GPR_rbx());
    __ pushl(Encoded_GPR_rcx());
    __ pushl(Encoded_GPR_rdx());
    __ pushl(Encoded_GPR_rbp());
    __ pushl(Encoded_GPR_rdi());
    __ pushl(Encoded_GPR_rsi());
    __ pushl(Encoded_GPR_r8());
    __ pushl(Encoded_GPR_r10());
    __ pushl(Encoded_GPR_r11());
    __ pushl(Encoded_GPR_r12());
    __ pushl(Encoded_GPR_r13());
    __ pushl(Encoded_GPR_r14());
    __ pushl(Encoded_GPR_r15());

    __ mov(IceType_i32, Encoded_GPR_rax(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rbx(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rcx(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rdx(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rbp(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rdi(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_rsi(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r8(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r10(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r11(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r12(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r13(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r14(), Immediate(0x00));
    __ mov(IceType_i32, Encoded_GPR_r15(), Immediate(0x00));
  }

  void addEpilogue() {
    __ mov(IceType_i64, raxSlotAddress(), Encoded_GPR_rax());
    __ mov(IceType_i64, rbxSlotAddress(), Encoded_GPR_rbx());
    __ mov(IceType_i64, rcxSlotAddress(), Encoded_GPR_rcx());
    __ mov(IceType_i64, rdxSlotAddress(), Encoded_GPR_rdx());
    __ mov(IceType_i64, rdiSlotAddress(), Encoded_GPR_rdi());
    __ mov(IceType_i64, rsiSlotAddress(), Encoded_GPR_rsi());
    __ mov(IceType_i64, rbpSlotAddress(), Encoded_GPR_rbp());
    __ mov(IceType_i64, rspSlotAddress(), Encoded_GPR_rsp());
    __ mov(IceType_i64, r8SlotAddress(), Encoded_GPR_r8());
    __ mov(IceType_i64, r9SlotAddress(), Encoded_GPR_r9());
    __ mov(IceType_i64, r10SlotAddress(), Encoded_GPR_r10());
    __ mov(IceType_i64, r11SlotAddress(), Encoded_GPR_r11());
    __ mov(IceType_i64, r12SlotAddress(), Encoded_GPR_r12());
    __ mov(IceType_i64, r13SlotAddress(), Encoded_GPR_r13());
    __ mov(IceType_i64, r14SlotAddress(), Encoded_GPR_r14());
    __ mov(IceType_i64, r15SlotAddress(), Encoded_GPR_r15());
    __ movups(xmm0SlotAddress(), Encoded_Xmm_xmm0());
    __ movups(xmm1SlotAddress(), Encoded_Xmm_xmm1());
    __ movups(xmm2SlotAddress(), Encoded_Xmm_xmm2());
    __ movups(xmm3SlotAddress(), Encoded_Xmm_xmm3());
    __ movups(xmm4SlotAddress(), Encoded_Xmm_xmm4());
    __ movups(xmm5SlotAddress(), Encoded_Xmm_xmm5());
    __ movups(xmm6SlotAddress(), Encoded_Xmm_xmm6());
    __ movups(xmm7SlotAddress(), Encoded_Xmm_xmm7());
    __ movups(xmm8SlotAddress(), Encoded_Xmm_xmm8());
    __ movups(xmm9SlotAddress(), Encoded_Xmm_xmm9());
    __ movups(xmm10SlotAddress(), Encoded_Xmm_xmm10());
    __ movups(xmm11SlotAddress(), Encoded_Xmm_xmm11());
    __ movups(xmm12SlotAddress(), Encoded_Xmm_xmm12());
    __ movups(xmm13SlotAddress(), Encoded_Xmm_xmm13());
    __ movups(xmm14SlotAddress(), Encoded_Xmm_xmm14());
    __ movups(xmm15SlotAddress(), Encoded_Xmm_xmm15());

    __ popl(Encoded_GPR_r15());
    __ popl(Encoded_GPR_r14());
    __ popl(Encoded_GPR_r13());
    __ popl(Encoded_GPR_r12());
    __ popl(Encoded_GPR_r11());
    __ popl(Encoded_GPR_r10());
    __ popl(Encoded_GPR_r8());
    __ popl(Encoded_GPR_rsi());
    __ popl(Encoded_GPR_rdi());
    __ popl(Encoded_GPR_rbp());
    __ popl(Encoded_GPR_rdx());
    __ popl(Encoded_GPR_rcx());
    __ popl(Encoded_GPR_rbx());
    __ popl(Encoded_GPR_rax());
    __ popl(Encoded_GPR_r9());

    __ ret();
  }

  bool NeedsEpilogue;
  uint32_t NumAllocatedDwords;
};

} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice

#endif // ASSEMBLERX8664_TESTUTIL_H_
