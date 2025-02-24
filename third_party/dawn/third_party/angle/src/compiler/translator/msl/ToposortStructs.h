//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_TOPOSORTSTRUCTS_H_
#define COMPILER_TRANSLATOR_MSL_TOPOSORTSTRUCTS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Does a toposort on structs based on type dependencies.
// Struct type declarations are moved to the top of the root block.
[[nodiscard]] bool ToposortStructs(TCompiler &compiler,
                                   SymbolEnv &symbolEnv,
                                   TIntermBlock &root,
                                   ProgramPreludeConfig &ppc);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_TOPOSORTSTRUCTS_H_
