//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FlagSamplersForTexelFetch.h: finds all instances of texelFetch used with a static reference to a
// sampler uniform, and flag that uniform as having been used with texelFetch
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_FLAGSAMPLERSWITHTEXELFETCH_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_FLAGSAMPLERSWITHTEXELFETCH_H_

#include "GLSLANG/ShaderVars.h"
#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

// This flags all samplers which are statically accessed by a texelFetch invokation- that is, the
// sampler is used as a direct argument to the call to texelFetch. Dynamic accesses, or accesses
// with any amount of indirection, are not counted.
[[nodiscard]] bool FlagSamplersForTexelFetch(TCompiler *compiler,
                                             TIntermBlock *root,
                                             TSymbolTable *symbolTable,
                                             std::vector<sh::ShaderVariable> *uniforms);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_FLAGSAMPLERSWITHTEXELFETCH_H_
