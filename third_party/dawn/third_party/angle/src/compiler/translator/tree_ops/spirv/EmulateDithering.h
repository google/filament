//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateDithering: Adds dithering code to fragment shader outputs based on a specialization
// constant control value.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEDITHERING_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEDITHERING_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;
class SpecConst;
class DriverUniform;

[[nodiscard]] bool EmulateDithering(TCompiler *compiler,
                                    const ShCompileOptions &compileOptions,
                                    TIntermBlock *root,
                                    TSymbolTable *symbolTable,
                                    SpecConst *specConst,
                                    const DriverUniform *driverUniforms);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEDITHERING_H_
