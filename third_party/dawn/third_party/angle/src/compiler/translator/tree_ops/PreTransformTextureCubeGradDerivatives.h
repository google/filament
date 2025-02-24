//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Multiple GPU vendors have issues with transforming explicit cubemap
// derivatives onto the appropriate face. The workarounds are vendor-specific.

#ifndef COMPILER_TRANSLATOR_TREEOPS_PRETRANSFORMTEXTURECUBEGRADDERIVATIVES_H_
#define COMPILER_TRANSLATOR_TREEOPS_PRETRANSFORMTEXTURECUBEGRADDERIVATIVES_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// GLSL specs say the following regarding cube
// map sampling with explicit derivatives:
//
//   For the cube version, the partial derivatives of
//   P are assumed to be in the coordinate system used
//   before texture coordinates are projected onto the
//   appropriate cube face.
//
// Apple silicon expects them partially pre-projected
// onto the target face and written to certain vector
// components depending on the major axis.
[[nodiscard]] bool PreTransformTextureCubeGradDerivatives(TCompiler *compiler,
                                                          TIntermBlock *root,
                                                          TSymbolTable *symbolTable,
                                                          int shaderVersion);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_PRETRANSFORMTEXTURECUBEGRADDERIVATIVES_H_
