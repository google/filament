//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This mutating tree traversal flips the 2nd argument of interpolateAtOffset() to account for
// Y-coordinate flipping
//
// From: interpolateAtOffset(float interpolant, vec2 offset);
// To: interpolateAtOffset(float interpolant, vec2(offset * (pre-rotation * viewportYScale)));
//
// See http://anglebug.com/42262252

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITEINTERPOLATEATOFFSET_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITEINTERPOLATEATOFFSET_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class SpecConst;
class DriverUniform;

// If fragRotation = nullptr, no rotation will be applied.
[[nodiscard]] bool RewriteInterpolateAtOffset(TCompiler *compiler,
                                              TIntermBlock *root,
                                              TSymbolTable *symbolTable,
                                              int shaderVersion,
                                              SpecConst *specConst,
                                              const DriverUniform *driverUniforms);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITEINTERPOLATEATOFFSET_H_
