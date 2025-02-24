//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_EMITMETAL_H_
#define COMPILER_TRANSLATOR_MSL_EMITMETAL_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/RewritePipelines.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Walks the AST and emits Metal code.
[[nodiscard]] bool EmitMetal(TCompiler &compiler,
                             TIntermBlock &root,
                             IdGen &idGen,
                             const PipelineStructs &pipelineStructs,
                             SymbolEnv &symbolEnv,
                             const ProgramPreludeConfig &ppc,
                             const ShCompileOptions &compileOptions);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_EMITMETAL_H_
