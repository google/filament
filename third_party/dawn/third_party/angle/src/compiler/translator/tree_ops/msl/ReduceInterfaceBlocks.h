//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_REDUCEINTERFACEBLOCKS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_REDUCEINTERFACEBLOCKS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{
class TSymbolTable;

// This rewrites interface block declarations only.
//
// Access of interface blocks is not rewritten (e.g. TOperator::EOpIndexDirectInterfaceBlock). //
// XXX: ^ Still true?
//
// Example:
//  uniform Foo { int x; };
// Becomes:
//  uniform int x;
//
// Example:
//  uniform Foo { int x; } foo;
// Becomes:
//  struct Foo { int x; }; uniform Foo x;
//

[[nodiscard]] bool ReduceInterfaceBlocks(TCompiler &compiler, TIntermBlock &root, IdGen &idGen);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_REDUCEINTERFACEBLOCKS_H_
