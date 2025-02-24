//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteRepeatedAssignToSwizzled.h: Rewrite expressions that assign an assignment to a swizzled
// vector, like:
//     v.x = z = expression;
// to:
//     z = expression;
//     v.x = z;
//
// Note that this doesn't handle some corner cases: expressions nested inside other expressions,
// inside loop headers, or inside if conditions.

#ifndef COMPILER_TRANSLATOR_TREEOPS_GLSL_REWRITEREPEATEDASSIGNTOSWIZZLED_H_
#define COMPILER_TRANSLATOR_TREEOPS_GLSL_REWRITEREPEATEDASSIGNTOSWIZZLED_H_

#include "common/angleutils.h"
#include "common/debug.h"

namespace sh
{

class TCompiler;
class TIntermBlock;

#ifdef ANGLE_ENABLE_GLSL
[[nodiscard]] bool RewriteRepeatedAssignToSwizzled(TCompiler *compiler, TIntermBlock *root);
#else
[[nodiscard]] ANGLE_INLINE bool RewriteRepeatedAssignToSwizzled(TCompiler *compiler,
                                                                TIntermBlock *root)
{
    UNREACHABLE();
    return false;
}
#endif

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_GLSL_REWRITEREPEATEDASSIGNTOSWIZZLED_H_
