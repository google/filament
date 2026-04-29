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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_VERTEX_PULLING_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_VERTEX_PULLING_H_

#include "src/tint/api/common/vertex_pulling_config.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::transform {

/// The capabilities that the transform can support.
const Capabilities kVertexPullingCapabilities{};

/// This transform replaces vertex shader inputs with storage buffers, so that we can apply
/// robustness to the vertex input accesses.
///
/// We bind the storage buffers as arrays of u32, so any read to byte position `p` will actually
/// need to read position `p / 4`, since `sizeof(u32) == 4`.
///
/// The config specifies the input format for each attribute. This isn't related to the type of the
/// input in the shader. For example, `VertexFormat::kVec2F16` tells us that the buffer will
/// contain `f16` elements, to be read as `vec2`. In the shader, a user would declare a `vec2<f32>`
/// input to be able to use them. The conversion between `f16` and `f32` is handled by this
/// transform.
///
/// @param module the module to transform
/// @param config the vertex pulling configuration
/// @returns success or failure
Result<SuccessType> VertexPulling(core::ir::Module& module, const VertexPullingConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_VERTEX_PULLING_H_
