//===- subzero/crosstest/test_vector_ops_main.cpp - Driver for tests ------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting insertelement and extractelement operations
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_vector_ops.ll  --driver=test_vector_ops_main.cpp \
   --prefix=Subzero_ --output=test_vector_ops */

#include <cstring>
#include <iostream>
#include <limits>
#include <stdlib.h>

#include "test_vector_ops.h"

// Return a set of test vectors for the given vector type. Due to lack
// of an aligned allocator in C++, the returned value is allocated with
// posix_memalign() and should be freed with free().
template <typename T>
typename VectorOps<T>::Ty *getTestVectors(size_t &NumTestVectors) {
  typedef typename VectorOps<T>::Ty Ty;
  typedef typename VectorOps<T>::ElementTy ElementTy;

  Ty Zero;
  memset(&Zero, 0, sizeof(Zero));
  Ty Incr;
  // Note: The casts in the next two initializations are necessary,
  // since ElementTy isn't necessarily the type that the value is stored
  // in the vector.
  for (int I = 0; I < VectorOps<T>::NumElements; ++I)
    Incr[I] = (ElementTy)I;
  Ty Decr;
  for (int I = 0; I < VectorOps<T>::NumElements; ++I)
    Decr[I] = (ElementTy)-I;
  Ty Min;
  for (int I = 0; I < VectorOps<T>::NumElements; ++I)
    Min[I] = std::numeric_limits<ElementTy>::min();
  Ty Max;
  for (int I = 0; I < VectorOps<T>::NumElements; ++I)
    Max[I] = std::numeric_limits<ElementTy>::max();
  Ty TestVectors[] = {Zero, Incr, Decr, Min, Max};

  NumTestVectors = sizeof(TestVectors) / sizeof(Ty);

  const size_t VECTOR_ALIGNMENT = 16;
  void *Dest;
  if (posix_memalign(&Dest, VECTOR_ALIGNMENT, sizeof(TestVectors))) {
    std::cerr << "memory allocation error\n";
    abort();
  }

  memcpy(Dest, TestVectors, sizeof(TestVectors));

  return static_cast<Ty *>(Dest);
}

template <typename T>
void testInsertElement(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename VectorOps<T>::Ty Ty;
  typedef typename VectorOps<T>::ElementTy ElementTy;

  size_t NumTestVectors;
  Ty *TestVectors = getTestVectors<T>(NumTestVectors);

  ElementTy TestElements[] = {0, 1, std::numeric_limits<ElementTy>::min(),
                              std::numeric_limits<ElementTy>::max()};
  const size_t NumTestElements = sizeof(TestElements) / sizeof(ElementTy);

  for (size_t VI = 0; VI < NumTestVectors; ++VI) {
    Ty Vect = TestVectors[VI];
    for (size_t EI = 0; EI < NumTestElements; ++EI) {
      ElementTy Elt = TestElements[EI];
      for (size_t I = 0; I < VectorOps<T>::NumElements; ++I) {
        Ty ResultLlc = VectorOps<T>::insertelement(Vect, Elt, I);
        Ty ResultSz = VectorOps<T>::Subzero_insertelement(Vect, Elt, I);
        ++TotalTests;
        if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
          ++Passes;
        } else {
          ++Failures;
          std::cout << "insertelement<" << VectorOps<T>::TypeName << ">(Vect=";
          std::cout << vectAsString<T>(Vect)
                    << ", Element=" << (typename VectorOps<T>::CastTy)Elt
                    << ", Pos=" << I << ")\n";
          std::cout << "llc=" << vectAsString<T>(ResultLlc) << "\n";
          std::cout << "sz =" << vectAsString<T>(ResultSz) << "\n";
        }
      }
    }
  }

  free(TestVectors);
}

template <typename T>
void testExtractElement(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename VectorOps<T>::Ty Ty;
  typedef typename VectorOps<T>::ElementTy ElementTy;
  typedef typename VectorOps<T>::CastTy CastTy;

  size_t NumTestVectors;
  Ty *TestVectors = getTestVectors<T>(NumTestVectors);

  for (size_t VI = 0; VI < NumTestVectors; ++VI) {
    Ty Vect = TestVectors[VI];
    for (size_t I = 0; I < VectorOps<T>::NumElements; ++I) {
      CastTy ResultLlc = VectorOps<T>::extractelement(Vect, I);
      CastTy ResultSz = VectorOps<T>::Subzero_extractelement(Vect, I);
      ++TotalTests;
      if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "extractelement<" << VectorOps<T>::TypeName << ">(Vect=";
        std::cout << vectAsString<T>(Vect) << ", Pos=" << I << ")\n";
        std::cout << "llc=" << ResultLlc << "\n";
        std::cout << "sz =" << ResultSz << "\n";
      }
    }
  }

  free(TestVectors);
}

template <typename T>
void testShuffleVector(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename VectorOps<T>::Ty Ty;
  typedef typename VectorOps<T>::ElementTy ElementTy;

  size_t NumTestVectors;
  Ty *TestVectors = getTestVectors<T>(NumTestVectors);

  for (size_t VI = 0; VI < NumTestVectors; ++VI) {
    Ty Vect0 = TestVectors[VI];
    for (size_t VJ = 0; VJ < NumTestVectors; ++VJ) {
      Ty Vect1 = TestVectors[VJ];
      for (uint32_t Which = 0; Which < VectorOps<T>::shufflevector_count();
           ++Which) {
        Ty ResultLlc = VectorOps<T>::shufflevector(Vect0, Vect1, Which);
        Ty ResultSz = VectorOps<T>::Subzero_shufflevector(Vect0, Vect1, Which);
        ++TotalTests;
        if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
          ++Passes;
        } else {
          ++Failures;
          std::cout << "shufflevector<" << VectorOps<T>::TypeName << ">(Vect0=";
          std::cout << vectAsString<T>(Vect0)
                    << ", Vect1=" << vectAsString<T>(Vect1) << ", Which=" << VJ
                    << ")\n";
          std::cout << "llc=" << vectAsString<T>(ResultLlc) << "\n";
          std::cout << "sz =" << vectAsString<T>(ResultSz) << "\n";
        }
      }
    }
  }

  free(TestVectors);
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testInsertElement<v4i1>(TotalTests, Passes, Failures);
  testInsertElement<v8i1>(TotalTests, Passes, Failures);
  testInsertElement<v16i1>(TotalTests, Passes, Failures);
  testInsertElement<v16si8>(TotalTests, Passes, Failures);
  testInsertElement<v16ui8>(TotalTests, Passes, Failures);
  testInsertElement<v8si16>(TotalTests, Passes, Failures);
  testInsertElement<v8ui16>(TotalTests, Passes, Failures);
  testInsertElement<v4si32>(TotalTests, Passes, Failures);
  testInsertElement<v4ui32>(TotalTests, Passes, Failures);
  testInsertElement<v4f32>(TotalTests, Passes, Failures);

  testExtractElement<v4i1>(TotalTests, Passes, Failures);
  testExtractElement<v8i1>(TotalTests, Passes, Failures);
  testExtractElement<v16i1>(TotalTests, Passes, Failures);
  testExtractElement<v16si8>(TotalTests, Passes, Failures);
  testExtractElement<v16ui8>(TotalTests, Passes, Failures);
  testExtractElement<v8si16>(TotalTests, Passes, Failures);
  testExtractElement<v8ui16>(TotalTests, Passes, Failures);
  testExtractElement<v4si32>(TotalTests, Passes, Failures);
  testExtractElement<v4ui32>(TotalTests, Passes, Failures);
  testExtractElement<v4f32>(TotalTests, Passes, Failures);

  testShuffleVector<v4i1>(TotalTests, Passes, Failures);
  testShuffleVector<v8i1>(TotalTests, Passes, Failures);
  testShuffleVector<v16i1>(TotalTests, Passes, Failures);
  testShuffleVector<v16si8>(TotalTests, Passes, Failures);
  testShuffleVector<v16ui8>(TotalTests, Passes, Failures);
  testShuffleVector<v8si16>(TotalTests, Passes, Failures);
  testShuffleVector<v8ui16>(TotalTests, Passes, Failures);
  testShuffleVector<v4si32>(TotalTests, Passes, Failures);
  testShuffleVector<v4ui32>(TotalTests, Passes, Failures);
  testShuffleVector<v4f32>(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";

  return Failures;
}
