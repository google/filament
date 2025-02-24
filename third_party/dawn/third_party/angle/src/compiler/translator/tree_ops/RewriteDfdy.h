//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteDfdy: Transform dFdx and dFdy according to pre-rotation and viewport y-flip.

#ifndef COMPILER_TRANSLATOR_TREEOPS_REWRITEDFDY_H_
#define COMPILER_TRANSLATOR_TREEOPS_REWRITEDFDY_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class SpecConst;
class DriverUniform;

[[nodiscard]] bool RewriteDfdy(TCompiler *compiler,
                               TIntermBlock *root,
                               TSymbolTable *symbolTable,
                               int shaderVersion,
                               SpecConst *specConst,
                               const DriverUniform *driverUniforms);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REWRITEDFDY_H_
