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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// The capabilities that the transform can support.
///
/// Note: BindingRemapper is the transform that introduces duplicate
/// bindings, so in theory shouldn't need the capability to allow
/// them. Except that in the MSL backend BindingRemapper is invoked multiple
/// times, (FlattenBindings and Raise specifically), so may encounter IR with
/// duplicate bindings when called the second time.
// TODO(crbug.com/363031535): Remove kAllowDuplicateBindings when MSL no
// longer needs FlattenBindings. binding_remapper_fuzz.cc will need to be
// updated to have kAllowDuplicateBindings as a post-run capability.
const Capabilities kBindingRemapperCapabilities{Capability::kAllowDuplicateBindings};

/// BindingRemapper is a transform that remaps binding point indices and access controls.
/// @param module the module to transform
/// @param binding_points the remapping data
/// @returns success or failure
Result<SuccessType> BindingRemapper(
    Module& module,
    const std::unordered_map<BindingPoint, BindingPoint>& binding_points);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_
