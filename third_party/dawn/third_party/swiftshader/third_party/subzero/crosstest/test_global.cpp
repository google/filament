//===- subzero/crosstest/test_global.cpp - Global variable access tests ---===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation for crosstesting global variable access operations.
//
//===----------------------------------------------------------------------===//

#include <cstdlib>
#include <stdint.h>

#include "test_global.h"

// Note: The following take advantage of the fact that external global
// names are not mangled with the --prefix CL argument. Hence, they
// should have the same relocation value for both llc and Subzero.
extern uint8_t *ExternName1;
extern uint8_t *ExternName2;
extern uint8_t *ExternName3;
extern uint8_t *ExternName4;
extern uint8_t *ExternName5;

// Partially initialized array
int ArrayInitPartial[10] = {60, 70, 80, 90, 100};
int ArrayInitFull[] = {10, 20, 30, 40, 50};
const int ArrayConst[] = {-10, -20, -30};
static double ArrayDouble[10] = {0.5, 1.5, 2.5, 3.5};

static struct {
  int Array1[5];
  uint8_t *Pointer1;
  double Array2[3];
  uint8_t *Pointer2;
  struct {
    uint8_t *Pointer3;
    int Array1[3];
    uint8_t *Pointer4;
  } NestedStuff;
  uint8_t *Pointer5;
} StructEx = {
    {10, 20, 30, 40, 50},
    ExternName1,
    {0.5, 1.5, 2.5},
    ExternName4,
    {ExternName3, {1000, 1010, 1020}, ExternName2},
    ExternName5,
};

#define ARRAY(a)                                                               \
  { (uint8_t *)(a), sizeof(a) }

// Note: By embedding the array addresses in this table, we are indirectly
// testing relocations (i.e. getArray would return the wrong address if
// relocations are broken).
struct {
  uint8_t *ArrayAddress;
  size_t ArraySizeInBytes;
} Arrays[] = {
    ARRAY(ArrayInitPartial),
    ARRAY(ArrayInitFull),
    ARRAY(ArrayConst),
    ARRAY(ArrayDouble),
    {(uint8_t *)(ArrayInitPartial + 2),
     sizeof(ArrayInitPartial) - 2 * sizeof(int)},
    {(uint8_t *)(&StructEx), sizeof(StructEx)},
};
size_t NumArraysElements = sizeof(Arrays) / sizeof(*Arrays);

size_t getNumArrays() { return NumArraysElements; }

const uint8_t *getArray(size_t WhichArray, size_t &Len) {
  if (WhichArray >= NumArraysElements) {
    Len = -1;
    return NULL;
  }
  Len = Arrays[WhichArray].ArraySizeInBytes;
  return Arrays[WhichArray].ArrayAddress;
}
