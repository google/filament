//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEOUTARGS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEOUTARGS_H_

#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// e.g.:
//    /*void foo(out int x, inout int y)*/
//    foo(z, w);
// becomes
//    foo(Out(z), InOut(w));
// unless `z` and `w` are detected to never alias.
// The translated example effectively behaves the same as:
//    int _1;
//    int _2 = w;
//    foo(_1, _2);
//    z = _1;
//    w = _2;
[[nodiscard]] bool RewriteOutArgs(TCompiler &compiler, TIntermBlock &root, SymbolEnv &symbolEnv);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITEOUTARGS_H_
