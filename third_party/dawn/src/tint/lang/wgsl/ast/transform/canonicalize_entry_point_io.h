// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// CanonicalizeEntryPointIO is a transform used to rewrite shader entry point
/// interfaces into a form that the generators can handle. Each entry point
/// function is stripped of all shader IO attributes and wrapped in a function
/// that provides the shader interface.
/// The transform config determines whether to use global variables, structures,
/// or parameters for the shader inputs and outputs, and optionally adds
/// additional builtins to the shader interface.
///
/// Before:
/// ```
/// struct Locations{
///   @location(1) loc1 : f32;
///   @location(2) loc2 : vec4<u32>;
/// };
///
/// @fragment
/// fn frag_main(@builtin(position) coord : vec4<f32>,
///              locations : Locations) -> @location(0) f32 {
///   if (coord.w > 1.0) {
///     return 0.0;
///   }
///   var col : f32 = (coord.x * locations.loc1);
///   return col;
/// }
/// ```
///
/// After (using structures for all parameters):
/// ```
/// struct Locations{
///   loc1 : f32;
///   loc2 : vec4<u32>;
/// };
///
/// struct frag_main_in {
///   @builtin(position) coord : vec4<f32>;
///   @location(1) loc1 : f32;
///   @location(2) loc2 : vec4<u32>
/// };
///
/// struct frag_main_out {
///   @location(0) loc0 : f32;
/// };
///
/// fn frag_main_inner(coord : vec4<f32>,
///                    locations : Locations) -> f32 {
///   if (coord.w > 1.0) {
///     return 0.0;
///   }
///   var col : f32 = (coord.x * locations.loc1);
///   return col;
/// }
///
/// @fragment
/// fn frag_main(in : frag_main_in) -> frag_main_out {
///   let inner_retval = frag_main_inner(in.coord, Locations(in.loc1, in.loc2));
///   var wrapper_result : frag_main_out;
///   wrapper_result.loc0 = inner_retval;
///   return wrapper_result;
/// }
/// ```
///
/// @note Depends on the following transforms to have been run first:
/// * Unshadow
class CanonicalizeEntryPointIO final : public Castable<CanonicalizeEntryPointIO, Transform> {
  public:
    /// ShaderStyle is an enumerator of different ways to emit shader IO.
    enum class ShaderStyle {
        /// Target SPIR-V (using global variables).
        kSpirv,
        /// Target GLSL (using global variables).
        kGlsl,
        /// Target MSL (using non-struct function parameters for builtins).
        kMsl,
        /// Target HLSL (using structures for all IO).
        kHlsl,
    };

    /// Configuration options for the transform.
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param style the approach to use for emitting shader IO.
        /// @param sample_mask an optional sample mask to combine with shader masks
        /// @param emit_vertex_point_size `true` to generate a pointsize builtin
        /// @param polyfill_f16_io `true` to replace f16 types with f32 types
        explicit Config(ShaderStyle style,
                        uint32_t sample_mask = 0xFFFFFFFF,
                        bool emit_vertex_point_size = false,
                        bool polyfill_f16_io = false);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The approach to use for emitting shader IO.
        ShaderStyle shader_style = ShaderStyle::kSpirv;

        /// A fixed sample mask to combine into masks produced by fragment shaders.
        uint32_t fixed_sample_mask = 0xffffffff;

        /// Set to `true` to generate a pointsize builtin and have it set to 1.0
        /// from all vertex shaders in the module.
        bool emit_vertex_point_size = false;

        /// Set to `true` to replace f16 IO types with f32 types and convert them.
        bool polyfill_f16_io = false;

        /// Reflection for this struct
        TINT_REFLECT(Config,
                     shader_style,
                     fixed_sample_mask,
                     emit_vertex_point_size,
                     polyfill_f16_io);
    };

    /// HLSLWaveIntrinsic is an InternalAttribute that is used to decorate a stub function so that
    /// the HLSL backend transforms this into calls to Wave* intrinsic functions.
    class HLSLWaveIntrinsic final : public Castable<HLSLWaveIntrinsic, InternalAttribute> {
      public:
        /// Wave intrinsic op
        enum class Op {
            kWaveGetLaneIndex,
            kWaveGetLaneCount,
        };

        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param o the op of the wave intrinsic
        HLSLWaveIntrinsic(GenerationID pid, NodeID nid, Op o);
        /// Destructor
        ~HLSLWaveIntrinsic() override;

        /// @copydoc InternalAttribute::InternalName
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const HLSLWaveIntrinsic* Clone(CloneContext& ctx) const override;

        /// The op of the intrinsic
        const Op op;
    };

    /// HLSLClipDistance1 is an InternalAttribute that is used to represent `SV_ClipDistance1`.
    class HLSLClipDistance1 final : public Castable<HLSLClipDistance1, InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        HLSLClipDistance1(GenerationID pid, NodeID nid);
        /// Destructor
        ~HLSLClipDistance1() override;

        /// @copydoc InternalAttribute::InternalName
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const HLSLClipDistance1* Clone(CloneContext& ctx) const override;
    };

    /// Constructor
    CanonicalizeEntryPointIO();
    ~CanonicalizeEntryPointIO() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

namespace tint {

/// Reflection for ShaderStyle
TINT_REFLECT_ENUM_RANGE(ast::transform::CanonicalizeEntryPointIO::ShaderStyle, kSpirv, kHlsl);

}  // namespace tint

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
