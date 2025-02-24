//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_FIXTYPECONSTRUCTORS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_FIXTYPECONSTRUCTORS_H_
#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{
[[nodiscard]] bool FixTypeConstructors(TCompiler &compiler,
                                       SymbolEnv &SymbolEnv,
                                       TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_FIXTYPECONSTRUCTORS_H_
