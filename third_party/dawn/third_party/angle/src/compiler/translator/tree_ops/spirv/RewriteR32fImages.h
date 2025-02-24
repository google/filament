//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteR32fImages: Change images qualified with r32f to use r32ui instead.  The only supported
// operation on these images is imageAtomicExchange(), which works identically with r32ui.  This
// avoids requiring atomic operations support for the R32_FLOAT format in Vulkan.

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITER32FIMAGES_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITER32FIMAGES_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool RewriteR32fImages(TCompiler *compiler,
                                     TIntermBlock *root,
                                     TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_REWRITER32FIMAGES_H_
