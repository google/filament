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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// The capabilities that the transform can support.
const core::ir::Capabilities kDirectVariableAccessCapabilities{
    core::ir::Capability::kAllowClipDistancesOnF32,
};

/// DirectVariableAccessOptions adjusts the behaviour of the transform.
struct DirectVariableAccessOptions {
    /// If true, then 'private' sub-object pointer arguments will be transformed.
    bool transform_private = false;
    /// If true, then 'function' sub-object pointer arguments will be transformed.
    bool transform_function = false;
    /// If true, then 'handle' sub-object handle type arguments will be transformed.
    bool transform_handle = false;

    /// Reflection for this class
    TINT_REFLECT(DirectVariableAccessOptions, transform_private, transform_function);
};

/// DirectVariableAccess is a transform that transforms parameters in the 'storage',
/// 'uniform' and 'workgroup' address space so that they're accessed directly by the function,
/// instead of being passed by pointer. It will potentiall also transform `private`, `handle` or
/// `function` parameters depending on provided options.
///
/// DirectVariableAccess works by creating specializations of functions that have matching
/// parameters, one specialization for each argument's unique access chain 'shape' from a unique
/// variable. Calls to specialized functions are transformed so that the arguments are replaced with
/// an array of access-chain indices, and if the parameter is in the 'function' or 'private'
/// address space, also with a pointer to the root object.
///
/// @param module the module to transform
/// @param options the options
/// @returns error on failure
Result<SuccessType> DirectVariableAccess(Module& module,
                                         const DirectVariableAccessOptions& options);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
