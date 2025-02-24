//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveAtomicCounterBuiltins: Remove atomic counter builtins.
// Normally handled by RewriteAtomicCounters, but that is only invoked when
// atomic counters are actually in use. This pass removes the builtins and
// asserts no atomic counters are declared.

#ifndef COMPILER_TRANSLATOR_TREEOPS_REMOVEATOMICCOUNTERBUILTINS_H_
#define COMPILER_TRANSLATOR_TREEOPS_REMOVEATOMICCOUNTERBUILTINS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;

[[nodiscard]] bool RemoveAtomicCounterBuiltins(TCompiler *compiler, TIntermBlock *root);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REMOVEATOMICCOUNTERBUILTINS_H_
