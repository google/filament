//===- subzero/crosstest/test_select.h - Test prototypes -----*- C++ -*----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes for cross testing the select
// bitcode instruction.
//
//===----------------------------------------------------------------------===//

#include "vectors.h"

v4f32 select(v4si32 cond, v4f32 val1, v4f32 val2);
v4si32 select(v4si32 cond, v4si32 val1, v4si32 val2);
v4ui32 select(v4si32 cond, v4ui32 val1, v4ui32 val2);
v8si16 select(v8si16 cond, v8si16 val1, v8si16 val2);
v8ui16 select(v8si16 cond, v8ui16 val1, v8ui16 val2);
v16si8 select(v16si8 cond, v16si8 val1, v16si8 val2);
v16ui8 select(v16si8 cond, v16ui8 val1, v16ui8 val2);
v4si32 select_i1(v4si32 cond, v4si32 val1, v4si32 val2);
v8si16 select_i1(v8si16 cond, v8si16 val1, v8si16 val2);
v16si8 select_i1(v16si8 cond, v16si8 val1, v16si8 val2);
