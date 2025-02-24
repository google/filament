//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_WRAPMAIN_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_WRAPMAIN_H_

#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// Changes
//    void main(args) { main-body }
// To
//    void FRESH_NAME(args) { main-body }
//    void main(args) { FRESH_NAME(args); }
//
// This transformation is useful if the original `main` has multiple return paths because this
// reduces down to a single path in the new `main`. Nice for inserting cleanup code in `main`.
[[nodiscard]] bool WrapMain(TCompiler &compiler, IdGen &idGen, TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_WRAPMAIN_H_
