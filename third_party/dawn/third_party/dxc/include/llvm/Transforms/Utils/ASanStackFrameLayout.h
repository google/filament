//===- ASanStackFrameLayout.h - ComputeASanStackFrameLayout -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines ComputeASanStackFrameLayout and auxiliary data structs.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_UTILS_ASANSTACKFRAMELAYOUT_H
#define LLVM_TRANSFORMS_UTILS_ASANSTACKFRAMELAYOUT_H
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

class AllocaInst;

// These magic constants should be the same as in
// in asan_internal.h from ASan runtime in compiler-rt.
static const int kAsanStackLeftRedzoneMagic = 0xf1;
static const int kAsanStackMidRedzoneMagic = 0xf2;
static const int kAsanStackRightRedzoneMagic = 0xf3;

// Input/output data struct for ComputeASanStackFrameLayout.
struct ASanStackVariableDescription {
  const char *Name;  // Name of the variable that will be displayed by asan
                     // if a stack-related bug is reported.
  uint64_t Size;     // Size of the variable in bytes.
  size_t Alignment;  // Alignment of the variable (power of 2).
  AllocaInst *AI;    // The actual AllocaInst.
  size_t Offset;     // Offset from the beginning of the frame;
                     // set by ComputeASanStackFrameLayout.
};

// Output data struct for ComputeASanStackFrameLayout.
struct ASanStackFrameLayout {
  // Frame description, see DescribeAddressIfStack in ASan runtime.
  SmallString<64> DescriptionString;
  // The contents of the shadow memory for the stack frame that we need
  // to set at function entry.
  SmallVector<uint8_t, 64> ShadowBytes;
  size_t FrameAlignment;  // Alignment for the entire frame.
  size_t FrameSize;       // Size of the frame in bytes.
};

void ComputeASanStackFrameLayout(
    // The array of stack variables. The elements may get reordered and changed.
    SmallVectorImpl<ASanStackVariableDescription> &Vars,
    // AddressSanitizer's shadow granularity. Usually 8, may also be 16, 32, 64.
    size_t Granularity,
    // The minimal size of the left-most redzone (header).
    // At least 4 pointer sizes, power of 2, and >= Granularity.
    // The resulting FrameSize should be multiple of MinHeaderSize.
    size_t MinHeaderSize,
    // The result is put here.
    ASanStackFrameLayout *Layout);

} // llvm namespace

#endif  // LLVM_TRANSFORMS_UTILS_ASANSTACKFRAMELAYOUT_H
