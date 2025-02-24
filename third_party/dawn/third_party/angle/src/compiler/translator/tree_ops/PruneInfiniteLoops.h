//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneInfiniteLoops.h: Attempts to remove infinite loops, used with WebGL contexts.

#ifndef COMPILER_TRANSLATOR_TREEOPS_PRUNEINFINITELOOPS_H_
#define COMPILER_TRANSLATOR_TREEOPS_PRUNEINFINITELOOPS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool PruneInfiniteLoops(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable *symbolTable,
                                      bool *anyLoopsPruned);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_PRUNEINFINITELOOPS_H_
