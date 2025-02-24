//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateFragColorData: Turn gl_FragColor and gl_FragData into normal fragment shader outputs.
// These built-ins are not supported by Vulkan.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAGCOLORDATA_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAGCOLORDATA_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool EmulateFragColorData(TCompiler *compiler,
                                        TIntermBlock *root,
                                        TSymbolTable *symbolTable,
                                        bool hasGLSecondaryFragData);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAGCOLORDATA_H_
