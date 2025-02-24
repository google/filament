//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_FORCEPRECISIONQUALIFIER_H_
#define COMPILER_TRANSLATOR_TREEOPS_FORCEPRECISIONQUALIFIER_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
bool ForceShaderPrecisionToMediump(TIntermNode *root, TSymbolTable *symbolTable, GLenum shaderType);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_FORCEPRECISIONQUALIFIER_H_
