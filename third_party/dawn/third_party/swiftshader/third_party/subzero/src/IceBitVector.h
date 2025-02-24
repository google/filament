//===- subzero/src/IceBitVector.h - Inline bit vector. ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines and implements a bit vector classes.
///
/// SmallBitVector is a drop in replacement for llvm::SmallBitVector. It uses
/// inline storage, at the expense of limited, static size.
///
/// BitVector is a allocator aware version of llvm::BitVector. Its
/// implementation was copied ipsis literis from llvm.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEBITVECTOR_H
#define SUBZERO_SRC_ICEBITVECTOR_H

#include "IceMemory.h"
#include "IceOperand.h"

#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <memory>
#include <type_traits>
#include <utility>

namespace Ice {
class SmallBitVector {
public:
  using ElementType = uint64_t;
  static constexpr SizeT BitIndexSize = 6; // log2(NumBitsPerPos);
  static constexpr SizeT NumBitsPerPos = sizeof(ElementType) * CHAR_BIT;
  static_assert(1 << BitIndexSize == NumBitsPerPos, "Invalid BitIndexSize.");

  SmallBitVector(const SmallBitVector &BV) { *this = BV; }

  SmallBitVector &operator=(const SmallBitVector &BV) {
    if (&BV != this) {
      resize(BV.size());
      memcpy(Bits, BV.Bits, sizeof(Bits));
    }
    return *this;
  }

  SmallBitVector() { reset(); }

  explicit SmallBitVector(SizeT S) : SmallBitVector() {
    assert(S <= MaxBits);
    resize(S);
  }

  class Reference {
    Reference() = delete;

  public:
    Reference(const Reference &) = default;
    Reference &operator=(const Reference &Rhs) { return *this = (bool)Rhs; }
    Reference &operator=(bool t) {
      if (t) {
        *Data |= _1 << Bit;
      } else {
        *Data &= ~(_1 << Bit);
      }
      return *this;
    }
    operator bool() const { return (*Data & (_1 << Bit)) != 0; }

  private:
    friend class SmallBitVector;
    Reference(ElementType *D, SizeT B) : Data(D), Bit(B) {
      assert(B < NumBitsPerPos);
    }

    ElementType *const Data;
    const SizeT Bit;
  };

  Reference operator[](unsigned Idx) {
    assert(Idx < size());
    return Reference(Bits + (Idx >> BitIndexSize),
                     Idx & ((_1 << BitIndexSize) - 1));
  }

  bool operator[](unsigned Idx) const {
    assert(Idx < size());
    return Bits[Idx >> BitIndexSize] &
           (_1 << (Idx & ((_1 << BitIndexSize) - 1)));
  }

  int find_first() const { return find_first<0>(); }

  int find_next(unsigned Prev) const { return find_next<0>(Prev); }

  bool any() const {
    for (SizeT i = 0; i < BitsElements; ++i) {
      if (Bits[i]) {
        return true;
      }
    }
    return false;
  }

  SizeT size() const { return Size; }

  void resize(SizeT Size) {
    assert(Size <= MaxBits);
    this->Size = Size;
  }

  void reserve(SizeT Size) {
    assert(Size <= MaxBits);
    (void)Size;
  }

  void set(unsigned Idx) { (*this)[Idx] = true; }

  void set() {
    for (SizeT ii = 0; ii < size(); ++ii) {
      (*this)[ii] = true;
    }
  }

  SizeT count() const {
    SizeT Count = 0;
    for (SizeT i = 0; i < BitsElements; ++i) {
      Count += llvm::countPopulation(Bits[i]);
    }
    return Count;
  }

  SmallBitVector operator&(const SmallBitVector &Rhs) const {
    assert(size() == Rhs.size());
    SmallBitVector Ret(std::max(size(), Rhs.size()));
    for (SizeT i = 0; i < BitsElements; ++i) {
      Ret.Bits[i] = Bits[i] & Rhs.Bits[i];
    }
    return Ret;
  }

  SmallBitVector operator~() const {
    SmallBitVector Ret = *this;
    Ret.invert<0>();
    return Ret;
  }

  SmallBitVector &operator|=(const SmallBitVector &Rhs) {
    assert(size() == Rhs.size());
    resize(std::max(size(), Rhs.size()));
    for (SizeT i = 0; i < BitsElements; ++i) {
      Bits[i] |= Rhs.Bits[i];
    }
    return *this;
  }

  SmallBitVector operator|(const SmallBitVector &Rhs) const {
    assert(size() == Rhs.size());
    SmallBitVector Ret(std::max(size(), Rhs.size()));
    for (SizeT i = 0; i < BitsElements; ++i) {
      Ret.Bits[i] = Bits[i] | Rhs.Bits[i];
    }
    return Ret;
  }

  void reset() { memset(Bits, 0, sizeof(Bits)); }

  void reset(const SmallBitVector &Mask) {
    for (const auto V : RegNumBVIter(Mask)) {
      (*this)[unsigned(V)] = false;
    }
  }

private:
  // _1 is the constant 1 of type ElementType.
  static constexpr ElementType _1 = ElementType(1);

  static constexpr SizeT BitsElements = 2;
  ElementType Bits[BitsElements];

  // MaxBits is defined here because it needs Bits to be defined.
  static constexpr SizeT MaxBits = sizeof(SmallBitVector::Bits) * CHAR_BIT;
  static_assert(sizeof(SmallBitVector::Bits) == 16,
                "Bits must be 16 bytes wide.");
  SizeT Size = 0;

  template <SizeT Pos>
  typename std::enable_if<Pos == BitsElements, int>::type find_first() const {
    return -1;
  }

  template <SizeT Pos>
      typename std::enable_if <
      Pos<BitsElements, int>::type find_first() const {
    if (Bits[Pos] != 0) {
      return NumBitsPerPos * Pos + llvm::countTrailingZeros(Bits[Pos]);
    }
    return find_first<Pos + 1>();
  }

  template <SizeT Pos>
  typename std::enable_if<Pos == BitsElements, int>::type
  find_next(unsigned) const {
    return -1;
  }

  template <SizeT Pos>
      typename std::enable_if <
      Pos<BitsElements, int>::type find_next(unsigned Prev) const {
    if (Prev + 1 < (Pos + 1) * NumBitsPerPos) {
      const ElementType Mask =
          (ElementType(1) << ((Prev + 1) - Pos * NumBitsPerPos)) - 1;
      const ElementType B = Bits[Pos] & ~Mask;
      if (B != 0) {
        return NumBitsPerPos * Pos + llvm::countTrailingZeros(B);
      }
      Prev = (1 + Pos) * NumBitsPerPos - 1;
    }
    return find_next<Pos + 1>(Prev);
  }

  template <SizeT Pos>
  typename std::enable_if<Pos == BitsElements, void>::type invert() {}

  template <SizeT Pos>
      typename std::enable_if < Pos<BitsElements, void>::type invert() {
    if (size() < Pos * NumBitsPerPos) {
      Bits[Pos] = 0;
    } else if ((Pos + 1) * NumBitsPerPos < size()) {
      Bits[Pos] ^= ~ElementType(0);
    } else {
      const ElementType Mask =
          (ElementType(1) << (size() - (Pos * NumBitsPerPos))) - 1;
      Bits[Pos] ^= Mask;
    }
    invert<Pos + 1>();
  }
};

template <template <typename> class AT> class BitVectorTmpl {
  typedef unsigned long BitWord;
  using Allocator = AT<BitWord>;

  enum { BITWORD_SIZE = (unsigned)sizeof(BitWord) * CHAR_BIT };

  static_assert(BITWORD_SIZE == 64 || BITWORD_SIZE == 32,
                "Unsupported word size");

  BitWord *Bits;     // Actual bits.
  unsigned Size;     // Size of bitvector in bits.
  unsigned Capacity; // Size of allocated memory in BitWord.
  Allocator Alloc;

  uint64_t alignTo(uint64_t Value, uint64_t Align) {
#ifdef PNACL_LLVM
    return llvm::RoundUpToAlignment(Value, Align);
#else  // !PNACL_LLVM
    return llvm::alignTo(Value, Align);
#endif // !PNACL_LLVM
  }

public:
  typedef unsigned size_type;
  // Encapsulation of a single bit.
  class reference {
    friend class BitVectorTmpl;

    BitWord *WordRef;
    unsigned BitPos;

    reference(); // Undefined

  public:
    reference(BitVectorTmpl &b, unsigned Idx) {
      WordRef = &b.Bits[Idx / BITWORD_SIZE];
      BitPos = Idx % BITWORD_SIZE;
    }

    reference(const reference &) = default;

    reference &operator=(reference t) {
      *this = bool(t);
      return *this;
    }

    reference &operator=(bool t) {
      if (t)
        *WordRef |= BitWord(1) << BitPos;
      else
        *WordRef &= ~(BitWord(1) << BitPos);
      return *this;
    }

    operator bool() const {
      return ((*WordRef) & (BitWord(1) << BitPos)) ? true : false;
    }
  };

  /// BitVectorTmpl default ctor - Creates an empty bitvector.
  BitVectorTmpl(Allocator A = Allocator())
      : Size(0), Capacity(0), Alloc(std::move(A)) {
    Bits = nullptr;
  }

  /// BitVectorTmpl ctor - Creates a bitvector of specified number of bits. All
  /// bits are initialized to the specified value.
  explicit BitVectorTmpl(unsigned s, bool t = false, Allocator A = Allocator())
      : Size(s), Alloc(std::move(A)) {
    Capacity = NumBitWords(s);
    Bits = Alloc.allocate(Capacity);
    init_words(Bits, Capacity, t);
    if (t)
      clear_unused_bits();
  }

  /// BitVectorTmpl copy ctor.
  BitVectorTmpl(const BitVectorTmpl &RHS) : Size(RHS.size()), Alloc(RHS.Alloc) {
    if (Size == 0) {
      Bits = nullptr;
      Capacity = 0;
      return;
    }

    Capacity = NumBitWords(RHS.size());
    Bits = Alloc.allocate(Capacity);
    std::memcpy(Bits, RHS.Bits, Capacity * sizeof(BitWord));
  }

  BitVectorTmpl(BitVectorTmpl &&RHS)
      : Bits(RHS.Bits), Size(RHS.Size), Capacity(RHS.Capacity),
        Alloc(std::move(RHS.Alloc)) {
    RHS.Bits = nullptr;
  }

  ~BitVectorTmpl() {
    if (Bits != nullptr) {
      Alloc.deallocate(Bits, Capacity);
    }
  }

  /// empty - Tests whether there are no bits in this bitvector.
  bool empty() const { return Size == 0; }

  /// size - Returns the number of bits in this bitvector.
  size_type size() const { return Size; }

  /// count - Returns the number of bits which are set.
  size_type count() const {
    unsigned NumBits = 0;
    for (unsigned i = 0; i < NumBitWords(size()); ++i)
      NumBits += llvm::countPopulation(Bits[i]);
    return NumBits;
  }

  /// any - Returns true if any bit is set.
  bool any() const {
    for (unsigned i = 0; i < NumBitWords(size()); ++i)
      if (Bits[i] != 0)
        return true;
    return false;
  }

  /// all - Returns true if all bits are set.
  bool all() const {
    for (unsigned i = 0; i < Size / BITWORD_SIZE; ++i)
      if (Bits[i] != ~0UL)
        return false;

    // If bits remain check that they are ones. The unused bits are always zero.
    if (unsigned Remainder = Size % BITWORD_SIZE)
      return Bits[Size / BITWORD_SIZE] == (1UL << Remainder) - 1;

    return true;
  }

  /// none - Returns true if none of the bits are set.
  bool none() const { return !any(); }

  /// find_first - Returns the index of the first set bit, -1 if none
  /// of the bits are set.
  int find_first() const {
    for (unsigned i = 0; i < NumBitWords(size()); ++i)
      if (Bits[i] != 0)
        return i * BITWORD_SIZE + llvm::countTrailingZeros(Bits[i]);
    return -1;
  }

  /// find_next - Returns the index of the next set bit following the
  /// "Prev" bit. Returns -1 if the next set bit is not found.
  int find_next(unsigned Prev) const {
    ++Prev;
    if (Prev >= Size)
      return -1;

    unsigned WordPos = Prev / BITWORD_SIZE;
    unsigned BitPos = Prev % BITWORD_SIZE;
    BitWord Copy = Bits[WordPos];
    // Mask off previous bits.
    Copy &= ~0UL << BitPos;

    if (Copy != 0)
      return WordPos * BITWORD_SIZE + llvm::countTrailingZeros(Copy);

    // Check subsequent words.
    for (unsigned i = WordPos + 1; i < NumBitWords(size()); ++i)
      if (Bits[i] != 0)
        return i * BITWORD_SIZE + llvm::countTrailingZeros(Bits[i]);
    return -1;
  }

  /// clear - Clear all bits.
  void clear() { Size = 0; }

  /// resize - Grow or shrink the bitvector.
  void resize(unsigned N, bool t = false) {
    if (N > Capacity * BITWORD_SIZE) {
      unsigned OldCapacity = Capacity;
      grow(N);
      init_words(&Bits[OldCapacity], (Capacity - OldCapacity), t);
    }

    // Set any old unused bits that are now included in the BitVectorTmpl. This
    // may set bits that are not included in the new vector, but we will clear
    // them back out below.
    if (N > Size)
      set_unused_bits(t);

    // Update the size, and clear out any bits that are now unused
    unsigned OldSize = Size;
    Size = N;
    if (t || N < OldSize)
      clear_unused_bits();
  }

  void reserve(unsigned N) {
    if (N > Capacity * BITWORD_SIZE)
      grow(N);
  }

  // Set, reset, flip
  BitVectorTmpl &set() {
    init_words(Bits, Capacity, true);
    clear_unused_bits();
    return *this;
  }

  BitVectorTmpl &set(unsigned Idx) {
    assert(Bits && "Bits never allocated");
    Bits[Idx / BITWORD_SIZE] |= BitWord(1) << (Idx % BITWORD_SIZE);
    return *this;
  }

  /// set - Efficiently set a range of bits in [I, E)
  BitVectorTmpl &set(unsigned I, unsigned E) {
    assert(I <= E && "Attempted to set backwards range!");
    assert(E <= size() && "Attempted to set out-of-bounds range!");

    if (I == E)
      return *this;

    if (I / BITWORD_SIZE == E / BITWORD_SIZE) {
      BitWord EMask = 1UL << (E % BITWORD_SIZE);
      BitWord IMask = 1UL << (I % BITWORD_SIZE);
      BitWord Mask = EMask - IMask;
      Bits[I / BITWORD_SIZE] |= Mask;
      return *this;
    }

    BitWord PrefixMask = ~0UL << (I % BITWORD_SIZE);
    Bits[I / BITWORD_SIZE] |= PrefixMask;
    I = alignTo(I, BITWORD_SIZE);

    for (; I + BITWORD_SIZE <= E; I += BITWORD_SIZE)
      Bits[I / BITWORD_SIZE] = ~0UL;

    BitWord PostfixMask = (1UL << (E % BITWORD_SIZE)) - 1;
    if (I < E)
      Bits[I / BITWORD_SIZE] |= PostfixMask;

    return *this;
  }

  BitVectorTmpl &reset() {
    init_words(Bits, Capacity, false);
    return *this;
  }

  BitVectorTmpl &reset(unsigned Idx) {
    Bits[Idx / BITWORD_SIZE] &= ~(BitWord(1) << (Idx % BITWORD_SIZE));
    return *this;
  }

  /// reset - Efficiently reset a range of bits in [I, E)
  BitVectorTmpl &reset(unsigned I, unsigned E) {
    assert(I <= E && "Attempted to reset backwards range!");
    assert(E <= size() && "Attempted to reset out-of-bounds range!");

    if (I == E)
      return *this;

    if (I / BITWORD_SIZE == E / BITWORD_SIZE) {
      BitWord EMask = 1UL << (E % BITWORD_SIZE);
      BitWord IMask = 1UL << (I % BITWORD_SIZE);
      BitWord Mask = EMask - IMask;
      Bits[I / BITWORD_SIZE] &= ~Mask;
      return *this;
    }

    BitWord PrefixMask = ~0UL << (I % BITWORD_SIZE);
    Bits[I / BITWORD_SIZE] &= ~PrefixMask;
    I = alignTo(I, BITWORD_SIZE);

    for (; I + BITWORD_SIZE <= E; I += BITWORD_SIZE)
      Bits[I / BITWORD_SIZE] = 0UL;

    BitWord PostfixMask = (1UL << (E % BITWORD_SIZE)) - 1;
    if (I < E)
      Bits[I / BITWORD_SIZE] &= ~PostfixMask;

    return *this;
  }

  BitVectorTmpl &flip() {
    for (unsigned i = 0; i < NumBitWords(size()); ++i)
      Bits[i] = ~Bits[i];
    clear_unused_bits();
    return *this;
  }

  BitVectorTmpl &flip(unsigned Idx) {
    Bits[Idx / BITWORD_SIZE] ^= BitWord(1) << (Idx % BITWORD_SIZE);
    return *this;
  }

  // Indexing.
  reference operator[](unsigned Idx) {
    assert(Idx < Size && "Out-of-bounds Bit access.");
    return reference(*this, Idx);
  }

  bool operator[](unsigned Idx) const {
    assert(Idx < Size && "Out-of-bounds Bit access.");
    BitWord Mask = BitWord(1) << (Idx % BITWORD_SIZE);
    return (Bits[Idx / BITWORD_SIZE] & Mask) != 0;
  }

  bool test(unsigned Idx) const { return (*this)[Idx]; }

  /// Test if any common bits are set.
  bool anyCommon(const BitVectorTmpl &RHS) const {
    unsigned ThisWords = NumBitWords(size());
    unsigned RHSWords = NumBitWords(RHS.size());
    for (unsigned i = 0, e = std::min(ThisWords, RHSWords); i != e; ++i)
      if (Bits[i] & RHS.Bits[i])
        return true;
    return false;
  }

  // Comparison operators.
  bool operator==(const BitVectorTmpl &RHS) const {
    unsigned ThisWords = NumBitWords(size());
    unsigned RHSWords = NumBitWords(RHS.size());
    unsigned i;
    for (i = 0; i != std::min(ThisWords, RHSWords); ++i)
      if (Bits[i] != RHS.Bits[i])
        return false;

    // Verify that any extra words are all zeros.
    if (i != ThisWords) {
      for (; i != ThisWords; ++i)
        if (Bits[i])
          return false;
    } else if (i != RHSWords) {
      for (; i != RHSWords; ++i)
        if (RHS.Bits[i])
          return false;
    }
    return true;
  }

  bool operator!=(const BitVectorTmpl &RHS) const { return !(*this == RHS); }

  /// Intersection, union, disjoint union.
  BitVectorTmpl &operator&=(const BitVectorTmpl &RHS) {
    unsigned ThisWords = NumBitWords(size());
    unsigned RHSWords = NumBitWords(RHS.size());
    unsigned i;
    for (i = 0; i != std::min(ThisWords, RHSWords); ++i)
      Bits[i] &= RHS.Bits[i];

    // Any bits that are just in this bitvector become zero, because they aren't
    // in the RHS bit vector.  Any words only in RHS are ignored because they
    // are already zero in the LHS.
    for (; i != ThisWords; ++i)
      Bits[i] = 0;

    return *this;
  }

  /// reset - Reset bits that are set in RHS. Same as *this &= ~RHS.
  BitVectorTmpl &reset(const BitVectorTmpl &RHS) {
    unsigned ThisWords = NumBitWords(size());
    unsigned RHSWords = NumBitWords(RHS.size());
    unsigned i;
    for (i = 0; i != std::min(ThisWords, RHSWords); ++i)
      Bits[i] &= ~RHS.Bits[i];
    return *this;
  }

  /// test - Check if (This - RHS) is zero.
  /// This is the same as reset(RHS) and any().
  bool test(const BitVectorTmpl &RHS) const {
    unsigned ThisWords = NumBitWords(size());
    unsigned RHSWords = NumBitWords(RHS.size());
    unsigned i;
    for (i = 0; i != std::min(ThisWords, RHSWords); ++i)
      if ((Bits[i] & ~RHS.Bits[i]) != 0)
        return true;

    for (; i != ThisWords; ++i)
      if (Bits[i] != 0)
        return true;

    return false;
  }

  BitVectorTmpl &operator|=(const BitVectorTmpl &RHS) {
    if (size() < RHS.size())
      resize(RHS.size());
    for (size_t i = 0, e = NumBitWords(RHS.size()); i != e; ++i)
      Bits[i] |= RHS.Bits[i];
    return *this;
  }

  BitVectorTmpl &operator^=(const BitVectorTmpl &RHS) {
    if (size() < RHS.size())
      resize(RHS.size());
    for (size_t i = 0, e = NumBitWords(RHS.size()); i != e; ++i)
      Bits[i] ^= RHS.Bits[i];
    return *this;
  }

  // Assignment operator.
  const BitVectorTmpl &operator=(const BitVectorTmpl &RHS) {
    if (this == &RHS)
      return *this;

    Size = RHS.size();
    unsigned RHSWords = NumBitWords(Size);
    if (Size <= Capacity * BITWORD_SIZE) {
      if (Size)
        std::memcpy(Bits, RHS.Bits, RHSWords * sizeof(BitWord));
      clear_unused_bits();
      return *this;
    }

    // Currently, BitVectorTmpl is only used by liveness analysis.  With the
    // following assert, we make sure BitVectorTmpls grow in a single step from
    // 0 to their final capacity, rather than growing slowly and "leaking"
    // memory in the process.
    assert(Capacity == 0);

    // Grow the bitvector to have enough elements.
    const auto OldCapacity = Capacity;
    Capacity = RHSWords;
    assert(Capacity > 0 && "negative capacity?");
    BitWord *NewBits = Alloc.allocate(Capacity);
    std::memcpy(NewBits, RHS.Bits, Capacity * sizeof(BitWord));

    // Destroy the old bits.
    Alloc.deallocate(Bits, OldCapacity);
    Bits = NewBits;

    return *this;
  }

  const BitVectorTmpl &operator=(BitVectorTmpl &&RHS) {
    if (this == &RHS)
      return *this;

    Alloc.deallocate(Bits, Capacity);
    Bits = RHS.Bits;
    Size = RHS.Size;
    Capacity = RHS.Capacity;

    RHS.Bits = nullptr;

    return *this;
  }

  void swap(BitVectorTmpl &RHS) {
    std::swap(Bits, RHS.Bits);
    std::swap(Size, RHS.Size);
    std::swap(Capacity, RHS.Capacity);
  }

  //===--------------------------------------------------------------------===//
  // Portable bit mask operations.
  //===--------------------------------------------------------------------===//
  //
  // These methods all operate on arrays of uint32_t, each holding 32 bits. The
  // fixed word size makes it easier to work with literal bit vector constants
  // in portable code.
  //
  // The LSB in each word is the lowest numbered bit.  The size of a portable
  // bit mask is always a whole multiple of 32 bits.  If no bit mask size is
  // given, the bit mask is assumed to cover the entire BitVectorTmpl.

  /// setBitsInMask - Add '1' bits from Mask to this vector. Don't resize.
  /// This computes "*this |= Mask".
  void setBitsInMask(const uint32_t *Mask, unsigned MaskWords = ~0u) {
    applyMask<true, false>(Mask, MaskWords);
  }

  /// clearBitsInMask - Clear any bits in this vector that are set in Mask.
  /// Don't resize. This computes "*this &= ~Mask".
  void clearBitsInMask(const uint32_t *Mask, unsigned MaskWords = ~0u) {
    applyMask<false, false>(Mask, MaskWords);
  }

  /// setBitsNotInMask - Add a bit to this vector for every '0' bit in Mask.
  /// Don't resize.  This computes "*this |= ~Mask".
  void setBitsNotInMask(const uint32_t *Mask, unsigned MaskWords = ~0u) {
    applyMask<true, true>(Mask, MaskWords);
  }

  /// clearBitsNotInMask - Clear a bit in this vector for every '0' bit in Mask.
  /// Don't resize.  This computes "*this &= Mask".
  void clearBitsNotInMask(const uint32_t *Mask, unsigned MaskWords = ~0u) {
    applyMask<false, true>(Mask, MaskWords);
  }

private:
  unsigned NumBitWords(unsigned S) const {
    return (S + BITWORD_SIZE - 1) / BITWORD_SIZE;
  }

  // Set the unused bits in the high words.
  void set_unused_bits(bool t = true) {
    //  Set high words first.
    unsigned UsedWords = NumBitWords(Size);
    if (Capacity > UsedWords)
      init_words(&Bits[UsedWords], (Capacity - UsedWords), t);

    //  Then set any stray high bits of the last used word.
    unsigned ExtraBits = Size % BITWORD_SIZE;
    if (ExtraBits) {
      BitWord ExtraBitMask = ~0UL << ExtraBits;
      if (t)
        Bits[UsedWords - 1] |= ExtraBitMask;
      else
        Bits[UsedWords - 1] &= ~ExtraBitMask;
    }
  }

  // Clear the unused bits in the high words.
  void clear_unused_bits() { set_unused_bits(false); }

  void grow(unsigned NewSize) {
    const auto OldCapacity = Capacity;
    Capacity = std::max(NumBitWords(NewSize), Capacity * 2);
    assert(Capacity > 0 && "realloc-ing zero space");
    auto *NewBits = Alloc.allocate(Capacity);
    if (Bits) {
      std::memcpy(NewBits, Bits, OldCapacity * sizeof(BitWord));
      Alloc.deallocate(Bits, OldCapacity);
    }
    Bits = NewBits;

    clear_unused_bits();
  }

  void init_words(BitWord *B, unsigned NumWords, bool t) {
    memset(B, 0 - (int)t, NumWords * sizeof(BitWord));
  }

  template <bool AddBits, bool InvertMask>
  void applyMask(const uint32_t *Mask, unsigned MaskWords) {
    static_assert(BITWORD_SIZE % 32 == 0, "Unsupported BitWord size.");
    MaskWords = std::min(MaskWords, (size() + 31) / 32);
    const unsigned Scale = BITWORD_SIZE / 32;
    unsigned i;
    for (i = 0; MaskWords >= Scale; ++i, MaskWords -= Scale) {
      BitWord BW = Bits[i];
      // This inner loop should unroll completely when BITWORD_SIZE > 32.
      for (unsigned b = 0; b != BITWORD_SIZE; b += 32) {
        uint32_t M = *Mask++;
        if (InvertMask)
          M = ~M;
        if (AddBits)
          BW |= BitWord(M) << b;
        else
          BW &= ~(BitWord(M) << b);
      }
      Bits[i] = BW;
    }
    for (unsigned b = 0; MaskWords; b += 32, --MaskWords) {
      uint32_t M = *Mask++;
      if (InvertMask)
        M = ~M;
      if (AddBits)
        Bits[i] |= BitWord(M) << b;
      else
        Bits[i] &= ~(BitWord(M) << b);
    }
    if (AddBits)
      clear_unused_bits();
  }
};

using BitVector = BitVectorTmpl<CfgLocalAllocator>;

} // end of namespace Ice

namespace std {
/// Implement std::swap in terms of BitVectorTmpl swap.
template <template <typename> class AT>
inline void swap(Ice::BitVectorTmpl<AT> &LHS, Ice::BitVectorTmpl<AT> &RHS) {
  LHS.swap(RHS);
}
} // namespace std

#endif // SUBZERO_SRC_ICEBITVECTOR_H
