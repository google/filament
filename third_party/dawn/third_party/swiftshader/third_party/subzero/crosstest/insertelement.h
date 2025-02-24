//===- subzero/crosstest/insertelement.h - Helper for PNaCl workaround. ---===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Helper function to work around a potential stack overflow issue.
//
//===----------------------------------------------------------------------===//

#ifndef INSERTELEMENT_H
#define INSERTELEMENT_H

// Helper function to perform the insertelement bitcode instruction.  The PNaCl
// ABI simplifications transform insertelement/extractelement instructions with
// a non-constant index into something involving alloca.  This can cause a stack
// overflow if the alloca is inside a loop.
template <typename VectorType, typename ElementType>
void __attribute__((noinline))
setElement(VectorType &Value, size_t Index, ElementType Element) {
  Value[Index] = Element;
}

#endif // INSERTELEMENT_H
