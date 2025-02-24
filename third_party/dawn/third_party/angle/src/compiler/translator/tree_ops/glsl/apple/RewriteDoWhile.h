//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RewriteDoWhile.h: rewrite do-while loops as while loops to work around
// driver bugs

#ifndef COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_REWRITEDOWHILE_H_
#define COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_REWRITEDOWHILE_H_

#include "common/angleutils.h"
#include "common/debug.h"

namespace sh
{

class TCompiler;
class TIntermNode;
class TSymbolTable;

#if ANGLE_ENABLE_GLSL && ANGLE_PLATFORM_APPLE
[[nodiscard]] bool RewriteDoWhile(TCompiler *compiler,
                                  TIntermNode *root,
                                  TSymbolTable *symbolTable);
#else
[[nodiscard]] ANGLE_INLINE bool RewriteDoWhile(TCompiler *compiler,
                                               TIntermNode *root,
                                               TSymbolTable *symbolTable)
{
    UNREACHABLE();
    return false;
}
#endif

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_REWRITEDOWHILE_H_
