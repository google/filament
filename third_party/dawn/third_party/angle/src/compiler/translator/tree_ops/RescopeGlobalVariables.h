//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_RESCOPEGLOBALVARIABLES_H_
#define COMPILER_TRANSLATOR_TREEOPS_RESCOPEGLOBALVARIABLES_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

// This function rescopes any globally-declared variable to be local to a function if the variable
// is only used in one function.
[[nodiscard]] bool RescopeGlobalVariables(TCompiler &compiler, TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_RESCOPEGLOBALVARIABLES_H_
