//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_HLSL_AGGREGATEASSIGNARRAYSINSSBOS_H_
#define COMPILER_TRANSLATOR_TREEOPS_HLSL_AGGREGATEASSIGNARRAYSINSSBOS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool AggregateAssignArraysInSSBOs(TCompiler *compiler,
                                                TIntermBlock *root,
                                                TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_HLSL_AGGREGATEASSIGNARRAYSINSSBOS_H_
