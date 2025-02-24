//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteSampleMaskVariable.cpp: Find any references to gl_SampleMask and gl_SampleMaskIn, and
// rewrite it with ANGLESampleMask or ANGLESampleMaskIn.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class TIntermTyped;

// Rewrite every gl_SampleMask or gl_SampleMaskIn to "ANGLESampleMask" or "ANGLESampleMaskIn", then
// at the end of shader re-assign the values of this global variable to gl_SampleMask and
// gl_SampleMaskIn. This to solve the problem which the non constant index is used for the unsized
// array problem.
[[nodiscard]] bool RewriteSampleMask(TCompiler *compiler,
                                     TIntermBlock *root,
                                     TSymbolTable *symbolTable,
                                     const TIntermTyped *numSamplesUniform);

[[nodiscard]] bool RewriteSampleMaskIn(TCompiler *compiler,
                                       TIntermBlock *root,
                                       TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_
