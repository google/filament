//===- subzero/crosstest/vectors.h - Common SIMD vector utilies -*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides declarations for PNaCl portable SIMD vector types. In
// addition, this file provides utilies that may be useful for crosstesting
// vector code.
//
//===----------------------------------------------------------------------===//

#ifndef VECTORS_H
#define VECTORS_H

#include <sstream>
#include <stdint.h>
#include <string>

// The driver and the test program may be compiled by different
// versions of clang, with different standard libraries that have
// different definitions of int8_t.  Specifically, int8_t may be
// typedef'd as either 'char' or 'signed char', which mangle to
// different strings.  Avoid int8_t and use an explicit myint8_t.
typedef signed char myint8_t;

#include "vectors.def"

// PNaCl portable vector types
// Types declared: v4si32, v4ui32, v8si16, v8ui16, v16si8, v16ui8, v4f32
#define X(ty, eltty, castty) typedef eltty ty __attribute__((vector_size(16)));
VECTOR_TYPE_TABLE
#undef X

// i1 vector types are not native C++ SIMD vector types. Instead, for
// testing, they are expanded by the test code into native 128 bit
// SIMD vector types with the appropriate number of elements.
// Representing the types in Vectors<> requires a unique label for each
// type which this declaration provides.
// Types declared: v4i1, v8i1, v16i1
#define X(ty, expandedty, numelements) class ty;
I1_VECTOR_TYPE_TABLE
#undef X

namespace {

template <typename T> struct Vectors;

// Vectors<T> provides information about a vector type with label T:
// * Vectors<T>::Ty is the C++ vector type
// * Vectors<T>::ElementTy is the C++ element type
// * Vectors<T>::CastTy is a type that is safe to cast elements to and from
//       and is used for getting the representation of elements in ostreams
// * Vectors<T>::NumElements is the number of elements
// * Vectors<T>::TypeName is a string that names the type

#define DECLARE_VECTOR_TYPE(LABEL, TY, ELTTY, CASTTY, NUM_ELEMENTS)            \
  template <> struct Vectors<LABEL> {                                          \
    typedef TY Ty;                                                             \
    typedef ELTTY ElementTy;                                                   \
    typedef CASTTY CastTy;                                                     \
    static const size_t NumElements;                                           \
    static const char *const TypeName;                                         \
  };                                                                           \
  const size_t Vectors<LABEL>::NumElements = NUM_ELEMENTS;                     \
  const char *const Vectors<LABEL>::TypeName = #LABEL;

#define X(ty, eltty, castty)                                                   \
  DECLARE_VECTOR_TYPE(ty, ty, eltty, castty, (sizeof(ty) / sizeof(eltty)))
VECTOR_TYPE_TABLE
#undef X

#define X(ty, expandedty, numelements)                                         \
  DECLARE_VECTOR_TYPE(ty, expandedty, bool, int64_t, numelements)
I1_VECTOR_TYPE_TABLE
#undef X

#undef DECLARE_VECTOR_TYPE

// Return a string representation of the vector.
template <typename T>
std::string vectAsString(const typename Vectors<T>::Ty Vect) {
  std::ostringstream OS;
  for (size_t i = 0; i < Vectors<T>::NumElements; ++i) {
    if (i > 0)
      OS << " ";
    OS << (typename Vectors<T>::CastTy)Vect[i];
  }
  return OS.str();
}

// In some crosstests, test vectors are deterministically constructed by
// selecting elements from a pool of scalar values based on a
// pseudorandom sequence.  Testing all possible combinations of scalar
// values from the value pool is often not tractable.
//
// TODO: Replace with a portable PRNG from C++11.
class PRNG {
public:
  explicit PRNG(uint32_t Seed = 1) : State(Seed) {}

  uint32_t operator()() {
    // Lewis, Goodman, and Miller (1969)
    State = (16807 * State) % 2147483647;
    return State;
  }

private:
  uint32_t State;
};

} // end anonymous namespace

#endif // VECTORS_H
