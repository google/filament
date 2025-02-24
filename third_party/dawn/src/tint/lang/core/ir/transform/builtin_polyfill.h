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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_

#include <string>

#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// Enumerator of polyfill levels.
enum class BuiltinPolyfillLevel {
    /// No polyfill needed, supported by the backend.
    kNone,
    /// Clamp or range check the parameters.
    kClampOrRangeCheck,
    /// Polyfill the entire function.
    kFull,
};

/// The set of polyfills that should be applied.
struct BuiltinPolyfillConfig {
    /// Should `clamp()` be polyfilled for integer values?
    bool clamp_int = false;
    /// Should `countLeadingZeros()` be polyfilled?
    bool count_leading_zeros = false;
    /// Should `countTrailingZeros()` be polyfilled?
    bool count_trailing_zeros = false;
    /// Should `degrees()` be polyfilled?
    bool degrees = false;
    /// How should `extractBits()` be polyfilled?
    BuiltinPolyfillLevel extract_bits = BuiltinPolyfillLevel::kNone;
    /// Should `firstLeadingBit()` be polyfilled?
    bool first_leading_bit = false;
    /// Should `firstTrailingBit()` be polyfilled?
    bool first_trailing_bit = false;
    /// Should `fwidthFine()` be polyfilled?
    bool fwidth_fine = false;
    /// How should `insertBits()` be polyfilled?
    BuiltinPolyfillLevel insert_bits = BuiltinPolyfillLevel::kNone;
    /// Should `radians()` be polyfilled?
    bool radians = false;
    /// Should `reflect()` be polyfilled for vec2<f32>?
    bool reflect_vec2_f32 = false;
    /// Should `saturate()` be polyfilled?
    bool saturate = false;
    /// Should `textureSampleBaseClampToEdge()` be polyfilled for texture_2d<f32> textures?
    bool texture_sample_base_clamp_to_edge_2d_f32 = false;
    /// Should `dot4U8Packed()` and `dot4I8Packed()` be polyfilled?
    bool dot_4x8_packed = false;
    /// Should `pack4xI8()`, `pack4xU8()`, `pack4xI8Clamp()`, `unpack4xI8()` and `unpack4xU8()`
    /// be polyfilled?
    bool pack_unpack_4x8 = false;
    /// Should `pack4xU8Clamp()` be polyfilled?
    /// TODO(tint:1497): remove the option once the bug in DXC is fixed.
    bool pack_4xu8_clamp = false;
    /// Should `pack4x8snorm`, `pack4x8unorm`, `unpack4x8snorm` and `unpack4x8unorm` be polyfilled?
    bool pack_unpack_4x8_norm = false;

    /// Reflection for this class
    TINT_REFLECT(BuiltinPolyfillConfig,
                 clamp_int,
                 count_leading_zeros,
                 count_trailing_zeros,
                 extract_bits,
                 first_leading_bit,
                 first_trailing_bit,
                 insert_bits,
                 saturate,
                 texture_sample_base_clamp_to_edge_2d_f32,
                 dot_4x8_packed,
                 pack_unpack_4x8,
                 pack_4xu8_clamp);
};

/// BuiltinPolyfill is a transform that replaces calls to builtin functions and uses of other core
/// features with equivalent alternatives.
/// @param module the module to transform
/// @param config the polyfill configuration
/// @returns success or failure
Result<SuccessType> BuiltinPolyfill(Module& module, const BuiltinPolyfillConfig& config);

}  // namespace tint::core::ir::transform

namespace tint {

/// Reflection for BuiltinPolyfillLevel
TINT_REFLECT_ENUM_RANGE(tint::core::ir::transform::BuiltinPolyfillLevel, kNone, kFull);

}  // namespace tint

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
