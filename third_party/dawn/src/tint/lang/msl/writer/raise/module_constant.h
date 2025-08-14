// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_MSL_WRITER_RAISE_MODULE_CONSTANT_H_
#define SRC_TINT_LANG_MSL_WRITER_RAISE_MODULE_CONSTANT_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::msl::writer::raise {

/// The capabilities that the transform can support.
const core::ir::Capabilities kModuleConstantCapabilities{
    core::ir::Capability::kAllow8BitIntegers,
    core::ir::Capability::kAllow64BitIntegers,
    core::ir::Capability::kAllowPointersAndHandlesInStructures,
    core::ir::Capability::kAllowVectorElementPointer,
    core::ir::Capability::kAllowHandleVarsWithoutBindings,
    core::ir::Capability::kAllowClipDistancesOnF32,
    core::ir::Capability::kAllowPrivateVarsInFunctions,
    core::ir::Capability::kAllowAnyLetType,
    core::ir::Capability::kAllowNonCoreTypes,
    core::ir::Capability::kAllowWorkspacePointerInputToEntryPoint,
    core::ir::Capability::kAllowModuleScopeLets,
};

/// The set of polyfills that should be applied.
struct ModuleConstantConfig {
    // Set to true to disable module constant transform on constant data that has any f16.
    bool disable_module_constant_f16 = false;
};

/// ModuleConstant is a transform that moves all const data associated with access to a module scope
/// let. This transform is used to support 'program scope constants' in msl and thereby avoid the
/// potential for copying of large const in nested loops.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> ModuleConstant(core::ir::Module& module, const ModuleConstantConfig& config);

}  // namespace tint::msl::writer::raise

#endif  // SRC_TINT_LANG_MSL_WRITER_RAISE_MODULE_CONSTANT_H_
