// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_GLSL_WRITER_RAISE_TEXTURE_POLYFILL_H_
#define SRC_TINT_LANG_GLSL_WRITER_RAISE_TEXTURE_POLYFILL_H_

#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::glsl::writer::raise {

struct TexturePolyfillConfig {
    /// The binding point to use for placeholder samplers.
    BindingPoint placeholder_sampler_bind_point;

    /// Options used to map WGSL textureNumLevels/textureNumSamples builtins to internal uniform
    /// buffer values. If not specified, emits corresponding GLSL builtins
    /// textureQueryLevels/textureSamples directly.
    TextureBuiltinsFromUniformOptions texture_builtins_from_uniform;

    /// Reflection for this class
    TINT_REFLECT(TexturePolyfillConfig,
                 placeholder_sampler_bind_point,
                 texture_builtins_from_uniform);
};

/// TexturePolyfill is a transform that replaces textures, samplers and functions calls to make them
/// compatible with GLSL ES 3.10
///
/// @param module the module to transform
/// @param cfg the configuration
/// @returns success or failure
Result<SuccessType> TexturePolyfill(core::ir::Module& module, const TexturePolyfillConfig& cfg);

}  // namespace tint::glsl::writer::raise

#endif  // SRC_TINT_LANG_GLSL_WRITER_RAISE_TEXTURE_POLYFILL_H_
