//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OutputSPIRV: Generate SPIR-V from the AST.
//

#ifndef COMPILER_TRANSLATOR_SPIRV_OUTPUTSPIRV_H_
#define COMPILER_TRANSLATOR_SPIRV_OUTPUTSPIRV_H_

#include "common/hash_containers.h"
#include "compiler/translator/Compiler.h"

namespace sh
{
bool OutputSPIRV(TCompiler *compiler,
                 TIntermBlock *root,
                 const ShCompileOptions &compileOptions,
                 const angle::HashMap<int, uint32_t> &uniqueToSpirvIdMap,
                 uint32_t firstUnusedSpirvId);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_SPIRV_OUTPUTSPIRV_H_
