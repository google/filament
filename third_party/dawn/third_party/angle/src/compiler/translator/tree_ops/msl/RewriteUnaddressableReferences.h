//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEUNADDRESSABLEREFERENCES_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEUNADDRESSABLEREFERENCES_H_

#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Given:
//   void foo(out x);
// It is possible for the following to be legal in GLSL but not in Metal:
//   foo(expression);
// This can happen in cases where `expression` is a vector swizzle or vector element access.
// This rewrite functionality introduces temporaries that serve as proxies to be passed to the
// out/inout parameters as needed. The corresponding expressions get populated with their
// proxies after the function call.
[[nodiscard]] bool RewriteUnaddressableReferences(TCompiler &compiler,
                                                  TIntermBlock &root,
                                                  SymbolEnv &symbolEnv);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEUNADDRESSABLEREFERENCES_H_
