/// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_API_HELPERS_GENERATE_BINDINGS_H_
#define SRC_TINT_API_HELPERS_GENERATE_BINDINGS_H_

#include <string>
#include <unordered_set>

#include "src/tint/api/common/bindings.h"

// Forward declarations
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint {

/// Generate the resource bindings
/// @param module the module to generate from
/// @param set_group_to_zero if true, the group used for bindings will always be zero
/// @param flatten_bindings if true, the bindings will remap to count from 0
/// @param ycbcr_bindings flags a binding as YCBCR data.
/// @returns the bindings
Bindings GenerateBindings(const core::ir::Module& module,
                          const std::string& ep,
                          bool set_group_to_zero,
                          bool flatten_bindings,
                          std::unordered_set<tint::BindingPoint> ycbcr_bindings = {});

}  // namespace tint

#endif  // SRC_TINT_API_HELPERS_GENERATE_BINDINGS_H_
