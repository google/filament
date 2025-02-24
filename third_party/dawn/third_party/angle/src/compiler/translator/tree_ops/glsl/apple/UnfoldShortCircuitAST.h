//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitAST is an AST traverser to replace short-circuiting
// operations with ternary operations.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_UNFOLDSHORTCIRCUITAST_H_
#define COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_UNFOLDSHORTCIRCUITAST_H_

#include "common/angleutils.h"
#include "common/debug.h"

namespace sh
{

class TCompiler;
class TIntermBlock;

#if defined(ANGLE_ENABLE_GLSL) && defined(ANGLE_PLATFORM_APPLE)
[[nodiscard]] bool UnfoldShortCircuitAST(TCompiler *compiler, TIntermBlock *root);
#else
[[nodiscard]] ANGLE_INLINE bool UnfoldShortCircuitAST(TCompiler *compiler, TIntermBlock *root)
{
    UNREACHABLE();
    return false;
}
#endif

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_GLSL_APPLE_UNFOLDSHORTCIRCUITAST_H_
