//===- subzero/unittest/unittest/AssemblerX8632/TestUtil.h ------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Utility classes for testing the X8632 Assembler.
//
//===----------------------------------------------------------------------===//

#ifndef ASSEMBLERX8632_TESTUTIL_H_
#define ASSEMBLERX8632_TESTUTIL_H_

#include "IceAssemblerX8632.h"
#include "IceDefs.h"

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
namespace X8632 {
namespace Test {

class AssemblerX8632TestBase : public ::testing::Test {
protected:
  using Address = AssemblerX8632::Traits::Address;
  using Cond = AssemblerX8632::CondX86;
  using GPRRegister = AssemblerX8632::Traits::GPRRegister;
  using ByteRegister = AssemblerX8632::Traits::ByteRegister;
  using Label = ::Ice::X8632::Label;
  using Traits = AssemblerX8632::Traits;
  using XmmRegister = AssemblerX8632::Traits::XmmRegister;
  using X87STRegister = AssemblerX8632::Traits::X87STRegister;

  AssemblerX8632TestBase() { reset(); }

  void reset() { Assembler = makeUnique<AssemblerX8632>(); }

  AssemblerX8632 *assembler() const { return Assembler.get(); }

  size_t codeBytesSize() const { return Assembler->getBufferView().size(); }

  const uint8_t *codeBytes() const {
    return static_cast<const uint8_t *>(
        static_cast<const void *>(Assembler->getBufferView().data()));
  }

private:
  std::unique_ptr<AssemblerX8632> Assembler;
};

// __ is a helper macro. It allows test cases to emit X8632 assembly
// instructions with
//
//   __ mov(GPRRegister::Reg_Eax, 1);
//   __ ret();
//
// and so on. The idea of having this was "stolen" from dart's unit tests.
#define __ (this->assembler())->

// AssemblerX8632LowLevelTest verify that the "basic" instructions the tests
// rely on are encoded correctly. Therefore, instead of executing the assembled
// code, these tests will verify that the assembled bytes are sane.
class AssemblerX8632LowLevelTest : public AssemblerX8632TestBase {
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
// We can now dive into testing each emitting method in AssemblerX8632. Each
// test will emit some instructions for performing the test. The assembled
// instructions will operate in a "safe" environment. All x86-32 registers are
// spilled to the program stack, and the registers are then zeroed out, with the
// exception of %esp and %ebp.
//
// The jitted code and the unittest code will share the same stack. Therefore,
// test harnesses need to ensure it does not leave anything it pushed on the
// stack.
//
// %ebp is initialized with a pointer for rIP-based addressing. This pointer is
// used for position-independent access to a scratchpad area for use in tests.
// This mechanism is used because the test framework needs to generate addresses
// that work on both x86-32 and x86-64 hosts, but are encodable using our x86-32
// assembler. This is made possible because the encoding for
//
//    pushq %rax (x86-64 only)
//
// is the same as the one for
//
//    pushl %eax (x86-32 only; not encodable in x86-64)
//
// Likewise, the encodings for
//
//    movl offset(%ebp), %reg (32-bit only)
//    movl <src>, offset(%ebp) (32-bit only)
//
// and
//
//    movl offset(%rbp), %reg (64-bit only)
//    movl <src>, offset(%rbp) (64-bit only)
//
// are also the same.
//
// We use a call instruction in order to generate a natural sized address on the
// stack. Said address is then removed from the stack with a pop %rBP, which can
// then be used to address memory safely in either x86-32 or x86-64, as long as
// the test code does not perform any arithmetic operation that writes to %rBP.
// This PC materialization technique is very common in x86-32 PIC.
//
// %rBP is used to provide the tests with a scratchpad area that can safely and
// portably be written to and read from. This scratchpad area is also used to
// store the "final" values in eax, ebx, ecx, edx, esi, and edi, allowing the
// harnesses access to 6 "return values" instead of the usual single return
// value supported by C++.
//
// The jitted code will look like the following:
//
// test:
//       push %eax
//       push %ebx
//       push %ecx
//       push %edx
//       push %edi
//       push %esi
//       push %ebp
//       call test$materialize_ip
// test$materialize_ip:                           <<------- %eBP will point here
//       pop  %ebp
//       mov  $0, %eax
//       mov  $0, %ebx
//       mov  $0, %ecx
//       mov  $0, %edx
//       mov  $0, %edi
//       mov  $0, %esi
//
//       << test code goes here >>
//
//       mov %eax, { 0 + $ScratchpadOffset}(%ebp)
//       mov %ebx, { 4 + $ScratchpadOffset}(%ebp)
//       mov %ecx, { 8 + $ScratchpadOffset}(%ebp)
//       mov %edx, {12 + $ScratchpadOffset}(%ebp)
//       mov %edi, {16 + $ScratchpadOffset}(%ebp)
//       mov %esi, {20 + $ScratchpadOffset}(%ebp)
//       mov %ebp, {24 + $ScratchpadOffset}(%ebp)
//       mov %esp, {28 + $ScratchpadOffset}(%ebp)
//       movups %xmm0, {32 + $ScratchpadOffset}(%ebp)
//       movups %xmm1, {48 + $ScratchpadOffset}(%ebp)
//       movups %xmm2, {64 + $ScratchpadOffset}(%ebp)
//       movusp %xmm3, {80 + $ScratchpadOffset}(%ebp)
//       movusp %xmm4, {96 + $ScratchpadOffset}(%ebp)
//       movusp %xmm5, {112 + $ScratchpadOffset}(%ebp)
//       movusp %xmm6, {128 + $ScratchpadOffset}(%ebp)
//       movusp %xmm7, {144 + $ScratchpadOffset}(%ebp)
//
//       pop %ebp
//       pop %esi
//       pop %edi
//       pop %edx
//       pop %ecx
//       pop %ebx
//       pop %eax
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
class AssemblerX8632Test : public AssemblerX8632TestBase {
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
    // The AssemblerX8632Test class needs to be a friend so that it can create
    // PackedArith objects (see below.)
    friend class AssemblerX8632Test;

    explicit PackedArith(const Dqword &MyLhs) : Lhs(MyLhs) {}

    // Lhs can't be a & because operator~ returns a temporary object that needs
    // access to its own Dqword.
    const Dqword Lhs;
  };

  // Named constructor for PackedArith objects.
  template <typename C> static PackedArith<C> packedAs(const Dqword &D) {
    return PackedArith<C>(D);
  }

  AssemblerX8632Test() { reset(); }

  void reset() {
    AssemblerX8632TestBase::reset();

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
    static constexpr uint32_t EaxSlot = 0;
    static constexpr uint32_t EbxSlot = 1;
    static constexpr uint32_t EcxSlot = 2;
    static constexpr uint32_t EdxSlot = 3;
    static constexpr uint32_t EdiSlot = 4;
    static constexpr uint32_t EsiSlot = 5;
    static constexpr uint32_t EbpSlot = 6;
    static constexpr uint32_t EspSlot = 7;
    // save 4 dwords for each xmm registers.
    static constexpr uint32_t Xmm0Slot = 8;
    static constexpr uint32_t Xmm1Slot = 12;
    static constexpr uint32_t Xmm2Slot = 16;
    static constexpr uint32_t Xmm3Slot = 20;
    static constexpr uint32_t Xmm4Slot = 24;
    static constexpr uint32_t Xmm5Slot = 28;
    static constexpr uint32_t Xmm6Slot = 32;
    static constexpr uint32_t Xmm7Slot = 36;
    static constexpr uint32_t ScratchpadSlots = 40;

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
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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
#endif
        ExecutableData = nullptr;
      }
    }

    void run() const { reinterpret_cast<void (*)()>(ExecutableData)(); }

    uint32_t eax() const { return contentsOfDword(AssembledTest::EaxSlot); }

    uint32_t ebx() const { return contentsOfDword(AssembledTest::EbxSlot); }

    uint32_t ecx() const { return contentsOfDword(AssembledTest::EcxSlot); }

    uint32_t edx() const { return contentsOfDword(AssembledTest::EdxSlot); }

    uint32_t edi() const { return contentsOfDword(AssembledTest::EdiSlot); }

    uint32_t esi() const { return contentsOfDword(AssembledTest::EsiSlot); }

    uint32_t ebp() const { return contentsOfDword(AssembledTest::EbpSlot); }

    uint32_t esp() const { return contentsOfDword(AssembledTest::EspSlot); }

    template <typename T> T xmm0() const {
      return xmm<T>(AssembledTest::Xmm0Slot);
    }

    template <typename T> T xmm1() const {
      return xmm<T>(AssembledTest::Xmm1Slot);
    }

    template <typename T> T xmm2() const {
      return xmm<T>(AssembledTest::Xmm2Slot);
    }

    template <typename T> T xmm3() const {
      return xmm<T>(AssembledTest::Xmm3Slot);
    }

    template <typename T> T xmm4() const {
      return xmm<T>(AssembledTest::Xmm4Slot);
    }

    template <typename T> T xmm5() const {
      return xmm<T>(AssembledTest::Xmm5Slot);
    }

    template <typename T> T xmm6() const {
      return xmm<T>(AssembledTest::Xmm6Slot);
    }

    template <typename T> T xmm7() const {
      return xmm<T>(AssembledTest::Xmm7Slot);
    }

    // contentsOfDword is used for reading the values in the scratchpad area.
    // Valid arguments are the dword ids returned by
    // AssemblerX8632Test::allocateDword() -- other inputs are considered
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
    return Address(GPRRegister::Encoded_Reg_ebp, dwordDisp(Dword), nullptr);
  }

private:
  // e??SlotAddress returns an AssemblerX8632::Traits::Address that can be used
  // by the test cases to encode an address operand for accessing the slot for
  // the specified register. These are all private for, when jitting the test
  // code, tests should not tamper with these values. Besides, during the test
  // execution these slots' contents are undefined and should not be accessed.
  Address eaxSlotAddress() { return dwordAddress(AssembledTest::EaxSlot); }
  Address ebxSlotAddress() { return dwordAddress(AssembledTest::EbxSlot); }
  Address ecxSlotAddress() { return dwordAddress(AssembledTest::EcxSlot); }
  Address edxSlotAddress() { return dwordAddress(AssembledTest::EdxSlot); }
  Address ediSlotAddress() { return dwordAddress(AssembledTest::EdiSlot); }
  Address esiSlotAddress() { return dwordAddress(AssembledTest::EsiSlot); }
  Address ebpSlotAddress() { return dwordAddress(AssembledTest::EbpSlot); }
  Address espSlotAddress() { return dwordAddress(AssembledTest::EspSlot); }
  Address xmm0SlotAddress() { return dwordAddress(AssembledTest::Xmm0Slot); }
  Address xmm1SlotAddress() { return dwordAddress(AssembledTest::Xmm1Slot); }
  Address xmm2SlotAddress() { return dwordAddress(AssembledTest::Xmm2Slot); }
  Address xmm3SlotAddress() { return dwordAddress(AssembledTest::Xmm3Slot); }
  Address xmm4SlotAddress() { return dwordAddress(AssembledTest::Xmm4Slot); }
  Address xmm5SlotAddress() { return dwordAddress(AssembledTest::Xmm5Slot); }
  Address xmm6SlotAddress() { return dwordAddress(AssembledTest::Xmm6Slot); }
  Address xmm7SlotAddress() { return dwordAddress(AssembledTest::Xmm7Slot); }

  // Returns the displacement that should be used when accessing the specified
  // Dword in the scratchpad area. It needs to adjust for the initial
  // instructions that are emitted before the call that materializes the IP
  // register.
  uint32_t dwordDisp(uint32_t Dword) const {
    EXPECT_LT(Dword, NumAllocatedDwords);
    assert(Dword < NumAllocatedDwords);
    static constexpr uint8_t PushBytes = 1;
    static constexpr uint8_t CallImmBytes = 5;
    return AssembledTest::MaximumCodeSize + (Dword * 4) -
           (7 * PushBytes + CallImmBytes);
  }

  void addPrologue() {
    __ pushl(GPRRegister::Encoded_Reg_eax);
    __ pushl(GPRRegister::Encoded_Reg_ebx);
    __ pushl(GPRRegister::Encoded_Reg_ecx);
    __ pushl(GPRRegister::Encoded_Reg_edx);
    __ pushl(GPRRegister::Encoded_Reg_edi);
    __ pushl(GPRRegister::Encoded_Reg_esi);
    __ pushl(GPRRegister::Encoded_Reg_ebp);

    __ call(Immediate(4));
    __ popl(GPRRegister::Encoded_Reg_ebp);
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_eax, Immediate(0x00));
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_ebx, Immediate(0x00));
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_ecx, Immediate(0x00));
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_edx, Immediate(0x00));
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_edi, Immediate(0x00));
    __ mov(IceType_i32, GPRRegister::Encoded_Reg_esi, Immediate(0x00));
  }

  void addEpilogue() {
    __ mov(IceType_i32, eaxSlotAddress(), GPRRegister::Encoded_Reg_eax);
    __ mov(IceType_i32, ebxSlotAddress(), GPRRegister::Encoded_Reg_ebx);
    __ mov(IceType_i32, ecxSlotAddress(), GPRRegister::Encoded_Reg_ecx);
    __ mov(IceType_i32, edxSlotAddress(), GPRRegister::Encoded_Reg_edx);
    __ mov(IceType_i32, ediSlotAddress(), GPRRegister::Encoded_Reg_edi);
    __ mov(IceType_i32, esiSlotAddress(), GPRRegister::Encoded_Reg_esi);
    __ mov(IceType_i32, ebpSlotAddress(), GPRRegister::Encoded_Reg_ebp);
    __ mov(IceType_i32, espSlotAddress(), GPRRegister::Encoded_Reg_esp);
    __ movups(xmm0SlotAddress(), XmmRegister::Encoded_Reg_xmm0);
    __ movups(xmm1SlotAddress(), XmmRegister::Encoded_Reg_xmm1);
    __ movups(xmm2SlotAddress(), XmmRegister::Encoded_Reg_xmm2);
    __ movups(xmm3SlotAddress(), XmmRegister::Encoded_Reg_xmm3);
    __ movups(xmm4SlotAddress(), XmmRegister::Encoded_Reg_xmm4);
    __ movups(xmm5SlotAddress(), XmmRegister::Encoded_Reg_xmm5);
    __ movups(xmm6SlotAddress(), XmmRegister::Encoded_Reg_xmm6);
    __ movups(xmm7SlotAddress(), XmmRegister::Encoded_Reg_xmm7);

    __ popl(GPRRegister::Encoded_Reg_ebp);
    __ popl(GPRRegister::Encoded_Reg_esi);
    __ popl(GPRRegister::Encoded_Reg_edi);
    __ popl(GPRRegister::Encoded_Reg_edx);
    __ popl(GPRRegister::Encoded_Reg_ecx);
    __ popl(GPRRegister::Encoded_Reg_ebx);
    __ popl(GPRRegister::Encoded_Reg_eax);

    __ ret();
  }

  bool NeedsEpilogue;
  uint32_t NumAllocatedDwords;
};

} // end of namespace Test
} // end of namespace X8632
} // end of namespace Ice

#endif // ASSEMBLERX8632_TESTUTIL_H_
