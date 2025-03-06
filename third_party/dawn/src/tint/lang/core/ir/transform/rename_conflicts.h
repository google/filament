// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_RENAME_CONFLICTS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_RENAME_CONFLICTS_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// The capabilities that the transform can support.
const core::ir::Capabilities kRenameConflictsCapabilities{
    core::ir::Capability::kAllow8BitIntegers,
    core::ir::Capability::kAllow64BitIntegers,
    core::ir::Capability::kAllowPointersAndHandlesInStructures,
    core::ir::Capability::kAllowVectorElementPointer,
    core::ir::Capability::kAllowHandleVarsWithoutBindings,
    core::ir::Capability::kAllowClipDistancesOnF32,
    core::ir::Capability::kAllowPrivateVarsInFunctions,
    core::ir::Capability::kAllowAnyLetType,
};

/// RenameConflicts is a transform that renames declarations which prevent identifiers from
/// resolving to the correct declaration, and those with identical identifiers declared in the same
/// scope or a parent scope.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> RenameConflicts(core::ir::Module& module);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_RENAME_CONFLICTS_H_
