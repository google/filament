//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEINTERPOLANTS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEINTERPOLANTS_H_

#include "compiler/translator/msl/DriverUniformMetal.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// This transformation handles multisample interpolation semantics.
//
//  1. Types of all fragment inputs used with interpolation functions are adjusted with
//     an interpolant flag because MSL treats them as a separate type (Section 2.18).
//
//  2. Offset origin (0, 0) is in the pixel's upper-left corner in Metal but
//     in the pixel's center in OpenGL ES. Additionally, the Y direction may
//     be flipped depending on the bound FBO.
//
//  3. When a fragment input is used with any interpolation function, its regular usages
//     are wrapped with explicit interpolation functions based on the input's qualifier.
//
//  4. The outUsesSampleInterpolation variable is set to true if any fragment input
//     uses sample qualifier. This flag limits gl_SampleMaskIn built-in variable to
//     the current sample because Metal's [[sample_mask]] always contains all bits.
//
//  5. The outUsesSampleInterpolant variable is set to true if any fragment input that
//     has sample qualifier and is used as an argument to an interpolation function is
//     also used directly. This requires implicitly defining gl_SampleID.
[[nodiscard]] bool RewriteInterpolants(TCompiler &compiler,
                                       TIntermBlock &root,
                                       TSymbolTable &symbolTable,
                                       const DriverUniformMetal *driverUniforms,
                                       bool *outUsesSampleInterpolation,
                                       bool *outUsesSampleInterpolant);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEINTERPOLANTS_H_
