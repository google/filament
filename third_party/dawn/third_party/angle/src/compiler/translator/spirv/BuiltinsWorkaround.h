//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_SPIRV_BUILTINSWORKAROUND_H_
#define COMPILER_TRANSLATOR_SPIRV_BUILTINSWORKAROUND_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

#include "compiler/translator/Pragma.h"

namespace sh
{

[[nodiscard]] bool ShaderBuiltinsWorkaround(TCompiler *compiler,
                                            TIntermBlock *root,
                                            TSymbolTable *symbolTable,
                                            const ShCompileOptions &compileOptions);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_SPIRV_BUILTINSWORKAROUND_H_
