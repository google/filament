//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClampIndirectIndices.h: Add clamp to the indirect indices used on arrays.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_CLAMPINDIRECTINDICES_H_
#define COMPILER_TRANSLATOR_TREEOPS_CLAMPINDIRECTINDICES_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermNode;
class TSymbolTable;

[[nodiscard]] bool ClampIndirectIndices(TCompiler *compiler,
                                        TIntermNode *root,
                                        TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_CLAMPINDIRECTINDICES_H_
