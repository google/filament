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

#ifndef SRC_TINT_LANG_MSL_WRITER_RAISE_ARGUMENT_BUFFERS_H_
#define SRC_TINT_LANG_MSL_WRITER_RAISE_ARGUMENT_BUFFERS_H_

#include <unordered_map>
#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/msl/writer/common/options.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::msl::writer::raise {

// Configuration for argument buffers.
struct ArgumentBuffersConfig {
    // The set of bindings which should not go into argument buffers.
    std::unordered_set<tint::BindingPoint> skip_bindings{};

    /// Map from the group id to the argument buffer information for the group
    std::unordered_map<uint32_t, ArgumentBufferInfo> group_to_argument_buffer_info{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArgumentBuffersConfig, skip_bindings, group_to_argument_buffer_info);
};

/// ArgumentBuffers is a transform that replaces module-scope variables with entry-point
/// declarations that are wrapped in an argument buffer structure and passed to functions that need
/// them. Each bind group will have a separate buffer created.
///
/// @note must come before ModuleScopeVars
/// @note does not support multiple entry points
///
/// @param module the module to transform
/// @param cfg the argument buffer configuration
/// @returns success or failure
Result<SuccessType> ArgumentBuffers(core::ir::Module& module, const ArgumentBuffersConfig& cfg);

}  // namespace tint::msl::writer::raise

#endif  // SRC_TINT_LANG_MSL_WRITER_RAISE_ARGUMENT_BUFFERS_H_
