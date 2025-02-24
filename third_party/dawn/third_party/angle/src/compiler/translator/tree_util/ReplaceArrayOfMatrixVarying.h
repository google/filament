//
// Copyright 2020 The ANGLE Project Authors. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in the
// LICENSE file.
//
// ReplaceArrayOfMatrixVarying: Find any references to array of matrices varying
// and replace it with array of vectors.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REPLACEARRAYOFMATRIXVARYING_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REPLACEARRAYOFMATRIXVARYING_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class TVariable;

[[nodiscard]] bool ReplaceArrayOfMatrixVarying(TCompiler *compiler,
                                               TIntermBlock *root,
                                               TSymbolTable *symbolTable,
                                               const TVariable *varying);

[[nodiscard]] bool ReplaceArrayOfMatrixVaryings(TCompiler *compiler,
                                                TIntermBlock *root,
                                                TSymbolTable *symbolTable);
}  // namespace sh

#endif
