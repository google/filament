//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveInactiveInterfaceVariables.h:
//  Drop shader interface variable declarations for those that are inactive.  This step needs to be
//  done after CollectVariables.  This avoids having to emulate them (e.g. atomic counters for
//  Vulkan) or remove them in glslang wrapper (again, for Vulkan).
//
//  Shouldn't be run for the GL backend, as it queries shader interface variables from GL itself,
//  instead of relying on what was gathered during CollectVariables.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_REMOVEINACTIVEVARIABLES_H_
#define COMPILER_TRANSLATOR_TREEOPS_REMOVEINACTIVEVARIABLES_H_

#include "common/angleutils.h"

namespace sh
{

struct InterfaceBlock;
struct ShaderVariable;
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool RemoveInactiveInterfaceVariables(
    TCompiler *compiler,
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    const std::vector<sh::ShaderVariable> &attributes,
    const std::vector<sh::ShaderVariable> &inputVaryings,
    const std::vector<sh::ShaderVariable> &outputVariables,
    const std::vector<sh::ShaderVariable> &uniforms,
    const std::vector<sh::InterfaceBlock> &interfaceBlocks,
    bool removeFragmentOutputs);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REMOVEINACTIVEVARIABLES_H_
