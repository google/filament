//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_SEPARATECOMPOUNDEXPRESSIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_SEPARATECOMPOUNDEXPRESSIONS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Transforms code to (usually) have most one non-terminal expression per statement.
// This also rewrites (&&), (||), and (?:) into raw if/if-not/if-else statements, respectively.
//
// e.g.
//    int x = 6 + foo(y, bar());
// becomes
//    auto _1 = bar();
//    auto _2 = foo(y, _1);
//    auto _3 = 6 + _2;
//    int x = _3;
//
// WARNING:
//    - This does not rewrite object indexing operators as a whole (e.g. foo.x, foo[x]), but will
//      rewrite the arguments to the operator (when applicable).
//      e.g.
//        foo(getVec()[i + 2] + 1);
//      becomes
//        auto _1 = getVec();
//        auto _2 = i + 2;
//        auto _3 = _1[_2] + 1; // Index operator remains in (+) expr here.
//        foo(_3);
//
[[nodiscard]] bool SeparateCompoundExpressions(TCompiler &compiler,
                                               SymbolEnv &symbolEnv,
                                               IdGen &idGen,
                                               TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_SEPARATECOMPOUNDEXPRESSIONS_H_
