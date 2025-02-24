//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceClipCullDistanceVariable.h: Find any references to gl_ClipDistance or gl_CullDistance and
// replace it with ANGLEClipDistance or ANGLECullDistance.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REPLACECLIPCULLDISTANCEVARIABLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REPLACECLIPCULLDISTANCEVARIABLE_H_

#include "GLSLANG/ShaderLang.h"
#include "common/angleutils.h"

namespace sh
{

struct InterfaceBlock;
class TCompiler;
class TIntermBlock;
class TSymbolTable;
class TIntermTyped;

// Replace every gl_ClipDistance assignment with assignment to "ANGLEClipDistance",
// then at the end of shader re-assign the values of this global variable to gl_ClipDistance.
// This to solve some complex usages such as user passing gl_ClipDistance as output reference
// to a function.
// Furthermore, at the end shader, some disabled gl_ClipDistance[i] can be skipped from the
// assignment.
[[nodiscard]] bool ReplaceClipDistanceAssignments(TCompiler *compiler,
                                                  TIntermBlock *root,
                                                  TSymbolTable *symbolTable,
                                                  const GLenum shaderType,
                                                  const TIntermTyped *clipDistanceEnableFlags);

[[nodiscard]] bool ReplaceCullDistanceAssignments(TCompiler *compiler,
                                                  TIntermBlock *root,
                                                  TSymbolTable *symbolTable,
                                                  const GLenum shaderType);

[[nodiscard]] bool ZeroDisabledClipDistanceAssignments(TCompiler *compiler,
                                                       TIntermBlock *root,
                                                       TSymbolTable *symbolTable,
                                                       const GLenum shaderType,
                                                       const TIntermTyped *clipDistanceEnableFlags);
}  // namespace sh

#endif
