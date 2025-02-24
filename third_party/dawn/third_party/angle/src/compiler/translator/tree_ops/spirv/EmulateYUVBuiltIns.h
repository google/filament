//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateYUVBuiltIns: Adds functions that emulate yuv_2_rgb and rgb_2_yuv built-ins.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEYUVBUILTINS_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEYUVBUILTINS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool EmulateYUVBuiltIns(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEYUVBUILTINS_H_
