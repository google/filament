//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_ADDEXPLICITTYPECASTS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_ADDEXPLICITTYPECASTS_H_

#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Adds explicit type casts into the AST where casting is done implicitly.
[[nodiscard]] bool AddExplicitTypeCasts(TCompiler &compiler,
                                        TIntermBlock &root,
                                        SymbolEnv &symbolEnv,
                                        bool needsExplicitBoolCasts);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_ADDEXPLICITTYPECASTS_H_
