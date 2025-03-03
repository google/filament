//===- llvm/ADT/SmallVector.cpp - 'Normally small' vectors ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SmallVector class.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallVector.h"
using namespace llvm;

/// grow_pod - This is an implementation of the grow() method which only works
/// on POD-like datatypes and is out of line to reduce code duplication.
void SmallVectorBase::grow_pod(void *FirstEl, size_t MinSizeInBytes,
                               size_t TSize) {
  size_t CurSizeBytes = size_in_bytes();
  size_t NewCapacityInBytes = 2 * capacity_in_bytes() + TSize; // Always grow.
  if (NewCapacityInBytes < MinSizeInBytes)
    NewCapacityInBytes = MinSizeInBytes;

  // HLSL Change Begin - Use overridable operator new.
  void *NewElts = new char[NewCapacityInBytes];
  if (CurSizeBytes > 0)
    memcpy(NewElts, this->BeginX, CurSizeBytes);
  if (BeginX != nullptr && BeginX != FirstEl)
    delete[] (char*)this->BeginX;
  assert(NewElts && "Out of memory");
  // HLSL Change End - Use overridable operator new.

  this->EndX = (char*)NewElts+CurSizeBytes;
  this->BeginX = NewElts;
  this->CapacityX = (char*)this->BeginX + NewCapacityInBytes;
}
