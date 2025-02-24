//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_REWRITE_PIXELLOCALSTORAGE_H_
#define COMPILER_TRANSLATOR_TREEOPS_REWRITE_PIXELLOCALSTORAGE_H_

#include <GLSLANG/ShaderLang.h>

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;

// This mutating tree traversal rewrites high level ANGLE_shader_pixel_local_storage operations to
// the type of shader operations specified by ShPixelLocalStorageType, found in ShCompileOptions.
[[nodiscard]] bool RewritePixelLocalStorage(TCompiler *compiler,
                                            TIntermBlock *root,
                                            TSymbolTable &symbolTable,
                                            const ShCompileOptions &compileOptions,
                                            int shaderVersion);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REWRITE_PIXELLOCALSTORAGE_H_
