//===- subzero/src/IceUtils.h - Utility functions ---------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines some utility functions.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEUTILS_H
#define SUBZERO_SRC_ICEUTILS_H

#include <climits>
#include <cmath> // std::signbit()

namespace Ice {
namespace Utils {

/// Allows copying from types of unrelated sizes. This method was introduced to
/// enable the strict aliasing optimizations of GCC 4.4. Basically, GCC
/// mindlessly relies on obscure details in the C++ standard that make
/// reinterpret_cast virtually useless.
template <typename D, typename S> inline D bitCopy(const S &Source) {
  static_assert(sizeof(D) <= sizeof(S),
                "bitCopy between incompatible type widths");
  static_assert(!std::is_pointer<S>::value, "");
  D Destination;
  // This use of memcpy is safe: source and destination cannot overlap.
  memcpy(&Destination, reinterpret_cast<const void *>(&Source), sizeof(D));
  return Destination;
}

/// Check whether an N-bit two's-complement representation can hold value.
template <typename T> inline bool IsInt(int N, T value) {
  assert((0 < N) &&
         (static_cast<unsigned int>(N) < (CHAR_BIT * sizeof(value))));
  T limit = static_cast<T>(1) << (N - 1);
  return (-limit <= value) && (value < limit);
}

template <typename T> inline bool IsUint(int N, T value) {
  assert((0 < N) &&
         (static_cast<unsigned int>(N) < (CHAR_BIT * sizeof(value))));
  T limit = static_cast<T>(1) << N;
  return (0 <= value) && (value < limit);
}

/// Check whether the magnitude of value fits in N bits, i.e., whether an
/// (N+1)-bit sign-magnitude representation can hold value.
template <typename T> inline bool IsAbsoluteUint(int N, T Value) {
  assert((0 < N) &&
         (static_cast<unsigned int>(N) < (CHAR_BIT * sizeof(Value))));
  if (Value < 0)
    Value = -Value;
  return IsUint(N, Value);
}

/// Return true if the addition X + Y will cause integer overflow for integers
/// of type T.
template <typename T> inline bool WouldOverflowAdd(T X, T Y) {
  return ((X > 0 && Y > 0 && (X > std::numeric_limits<T>::max() - Y)) ||
          (X < 0 && Y < 0 && (X < std::numeric_limits<T>::min() - Y)));
}

/// Adds x to y and stores the result in sum. Returns true if the addition
/// overflowed.
inline bool add_overflow(uint32_t x, uint32_t y, uint32_t *sum) {
  static_assert(std::is_same<uint32_t, unsigned>::value, "Must match type");
#if __has_builtin(__builtin_uadd_overflow)
  return __builtin_uadd_overflow(x, y, sum);
#else
  *sum = x + y;
  return WouldOverflowAdd(x, y);
#endif
}

/// Return true if X is already aligned by N, where N is a power of 2.
template <typename T> inline bool IsAligned(T X, intptr_t N) {
  assert(llvm::isPowerOf2_64(N));
  return (X & (N - 1)) == 0;
}

/// Return Value adjusted to the next highest multiple of Alignment.
inline uint32_t applyAlignment(uint32_t Value, uint32_t Alignment) {
  assert(llvm::isPowerOf2_32(Alignment));
  return (Value + Alignment - 1) & -Alignment;
}

/// Return amount which must be added to adjust Pos to the next highest
/// multiple of Align.
inline uint64_t OffsetToAlignment(uint64_t Pos, uint64_t Align) {
  assert(llvm::isPowerOf2_64(Align));
  uint64_t Mod = Pos & (Align - 1);
  if (Mod == 0)
    return 0;
  return Align - Mod;
}

/// Rotate the value bit pattern to the left by shift bits.
/// Precondition: 0 <= shift < 32
inline uint32_t rotateLeft32(uint32_t value, uint32_t shift) {
  if (shift == 0)
    return value;
  return (value << shift) | (value >> (32 - shift));
}

/// Rotate the value bit pattern to the right by shift bits.
inline uint32_t rotateRight32(uint32_t value, uint32_t shift) {
  if (shift == 0)
    return value;
  return (value >> shift) | (value << (32 - shift));
}

/// Returns true if Val is +0.0. It requires T to be a floating point type.
template <typename T> bool isPositiveZero(T Val) {
  static_assert(std::is_floating_point<T>::value,
                "Input type must be floating point");
  return Val == 0 && !std::signbit(Val);
}

/// Resize a vector (or other suitable container) to a particular size, and also
/// reserve possibly a larger size to avoid repeatedly recopying as the
/// container grows.  It uses a strategy of doubling capacity up to a certain
/// point, after which it bumps the capacity by a fixed amount.
template <typename Container>
inline void reserveAndResize(Container &V, uint32_t Size,
                             uint32_t ChunkSizeBits = 10) {
#if __has_builtin(__builtin_clz)
  // Don't call reserve() if Size==0.
  if (Size > 0) {
    uint32_t Mask;
    if (Size <= (1u << ChunkSizeBits)) {
      // For smaller sizes, reserve the smallest power of 2 greater than or
      // equal to Size.
      Mask =
          ((1 << (CHAR_BIT * sizeof(uint32_t) - __builtin_clz(Size))) - 1) - 1;
    } else {
      // For larger sizes, round up to the smallest multiple of 1<<ChunkSizeBits
      // greater than or equal to Size.
      Mask = (1 << ChunkSizeBits) - 1;
    }
    V.reserve((Size + Mask) & ~Mask);
  }
#endif
  V.resize(Size);
}

/// An RAII class to ensure that a boolean flag is restored to its previous
/// value upon function exit.
///
/// Used in places like RandomizationPoolingPause and generating target helper
/// calls.
class BoolFlagSaver {
  BoolFlagSaver() = delete;
  BoolFlagSaver(const BoolFlagSaver &) = delete;
  BoolFlagSaver &operator=(const BoolFlagSaver &) = delete;

public:
  BoolFlagSaver(bool &F, bool NewValue) : OldValue(F), Flag(F) { F = NewValue; }
  ~BoolFlagSaver() { Flag = OldValue; }

private:
  const bool OldValue;
  bool &Flag;
};

} // end of namespace Utils
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEUTILS_H
