// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_BUILTIN_POLYFILL_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// Implements builtins for backends that do not have a native implementation.
class BuiltinPolyfill final : public Castable<BuiltinPolyfill, Transform> {
  public:
    /// Constructor
    BuiltinPolyfill();
    /// Destructor
    ~BuiltinPolyfill() override;

    /// Enumerator of polyfill levels
    enum class Level {
        /// No polyfill needed, supported by the backend.
        kNone,
        /// Clamp the parameters to the inner implementation.
        kClampParameters,
        /// Range check the input.
        kRangeCheck,
        /// Polyfill the entire function
        kFull,
    };

    /// Specifies the builtins that should be polyfilled by the transform.
    struct Builtins {
        /// What level should `acosh` be polyfilled?
        Level acosh = Level::kNone;
        /// Should `asinh` be polyfilled?
        bool asinh = false;
        /// What level should `atanh` be polyfilled?
        Level atanh = Level::kNone;
        /// Should storage textures of format 'bgra8unorm' be replaced with 'rgba8unorm'?
        bool bgra8unorm = false;
        /// Should the RHS of `<<` and `>>` be wrapped in a modulo bit-width of LHS?
        bool bitshift_modulo = false;
        /// Should `clamp()` be polyfilled for integer values (scalar or vector)?
        bool clamp_int = false;
        /// Should `countLeadingZeros()` be polyfilled?
        bool count_leading_zeros = false;
        /// Should `countTrailingZeros()` be polyfilled?
        bool count_trailing_zeros = false;
        /// Should converting f32 to i32 or u32 be polyfilled?
        bool conv_f32_to_iu32 = false;
        /// What level should `extractBits()` be polyfilled?
        Level extract_bits = Level::kNone;
        /// Should `firstLeadingBit()` be polyfilled?
        bool first_leading_bit = false;
        /// Should `firstTrailingBit()` be polyfilled?
        bool first_trailing_bit = false;
        /// Should `fwidthFine()` be polyfilled?
        bool fwidth_fine = false;
        /// Should `insertBits()` be polyfilled?
        Level insert_bits = Level::kNone;
        /// Should integer scalar / vector divides and modulos be polyfilled to avoid DBZ and
        /// integer overflows?
        bool int_div_mod = false;
        /// Should float modulos be polyfilled to emit a precise modulo operation as per the spec?
        bool precise_float_mod = false;
        /// Should `reflect()` be polyfilled for vec2<f32>?
        bool reflect_vec2_f32 = false;
        /// Should `saturate()` be polyfilled?
        bool saturate = false;
        /// Should `sign()` be polyfilled for integer types?
        bool sign_int = false;
        /// Should `textureSampleBaseClampToEdge()` be polyfilled for texture_2d<f32> textures?
        bool texture_sample_base_clamp_to_edge_2d_f32 = false;
        /// Should `workgroupUniformLoad()` be polyfilled?
        bool workgroup_uniform_load = false;
        /// Should `dot4I8Packed()` and `dot4U8Packed()` be polyfilled?
        bool dot_4x8_packed = false;
        /// Should `pack4xI8()`, `pack4xU8()`, `pack4xI8Clamp()`, `unpack4xI8()` and `unpack4xU8()`
        /// be polyfilled?
        bool pack_unpack_4x8 = false;
        /// Should `pack4xU8Clamp()` be polyfilled?
        /// TODO(tint:1497): remove the option once the bug in DXC is fixed.
        bool pack_4xu8_clamp = false;

        /// Reflection for this struct
        TINT_REFLECT(Builtins,
                     acosh,
                     asinh,
                     atanh,
                     bgra8unorm,
                     bitshift_modulo,
                     clamp_int,
                     count_leading_zeros,
                     count_trailing_zeros,
                     conv_f32_to_iu32,
                     extract_bits,
                     first_leading_bit,
                     first_trailing_bit,
                     fwidth_fine,
                     insert_bits,
                     int_div_mod,
                     precise_float_mod,
                     reflect_vec2_f32,
                     saturate,
                     sign_int,
                     texture_sample_base_clamp_to_edge_2d_f32,
                     workgroup_uniform_load,
                     dot_4x8_packed,
                     pack_unpack_4x8,
                     pack_4xu8_clamp);
    };

    /// Config is consumed by the BuiltinPolyfill transform.
    /// Config specifies the builtins that should be polyfilled.
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param b the list of builtins to polyfill
        explicit Config(const Builtins& b);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The builtins to polyfill
        Builtins builtins;

        /// Reflection for this struct
        TINT_REFLECT(Config, builtins);
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

namespace tint {

/// Level reflection information
TINT_REFLECT_ENUM_RANGE(ast::transform::BuiltinPolyfill::Level, kNone, kFull);

}  // namespace tint

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_BUILTIN_POLYFILL_H_
