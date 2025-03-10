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

#ifndef SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_TRUNCATE_INTERSTAGE_VARIABLES_H_
#define SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_TRUNCATE_INTERSTAGE_VARIABLES_H_

#include <bitset>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::hlsl::writer {

/// TruncateInterstageVariables is a transform that truncate interstage variables.
/// It must be run after CanonicalizeEntryPointIO which guarantees all interstage variables of
/// a given entry point are grouped into one shader IO struct.
/// It replaces `original shader IO struct` with a `new wrapper struct` containing builtin IOs
/// and user-defined IO whose locations are marked in the interstage_locations bitset from the
/// config. The return statements of `original shader IO struct` are wrapped by a mapping function
/// that initializes the members of `new wrapper struct` with values from `original shader IO
/// struct`. IO attributes of members in `original shader IO struct` are removed, other attributes
/// still preserve.
///
/// For example:
///
/// ```
///  struct ShaderIO {
///    @builtin(position) @invariant pos: vec4<f32>,
///    @location(1) f_1: f32,
///    @location(3) @align(16) f_3: f32,
///    @location(5) @interpolate(flat) @align(16) @size(16) f_5: u32,
///  }
///  @vertex
///  fn f() -> ShaderIO {
///    var io: ShaderIO;
///    io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
///    io.f_1 = 1.0;
///    io.f_3 = io.f_1 + 3.0;
///    io.f_5 = 1u;
///    return io;
///  }
/// ```
///
/// With config.interstage_locations[3] and [5] set to true, is transformed to:
///
/// ```
///  struct tint_symbol {
///    @builtin(position) @invariant
///    pos : vec4<f32>,
///    @location(3) @align(16)
///    f_3 : f32,
///    @location(5) @interpolate(flat) @align(16) @size(16)
///    f_5 : u32,
///  }
///
///  fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
///    return tint_symbol(io.pos, io.f_3, io.f_5);
///  }
///
///  struct ShaderIO {
///    pos : vec4<f32>,
///    f_1 : f32,
///    @align(16)
///    f_3 : f32,
///    @align(16) @size(16)
///    f_5 : u32,
///  }
///
///  @vertex
///  fn f() -> tint_symbol {
///    var io : ShaderIO;
///    io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
///    io.f_1 = 1.0;
///    io.f_3 = (io.f_1 + 3.0);
///    io.f_5 = 1u;
///    return truncate_shader_output(io);
///  }
/// ```
///
class TruncateInterstageVariables final
    : public Castable<TruncateInterstageVariables, ast::transform::Transform> {
  public:
    /// Configuration options for the transform
    struct Config final : public Castable<Config, ast::transform::Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// Indicate which interstage io locations are actually used by the later stage.
        /// There can be at most 30 user defined interstage variables with locations.
        std::bitset<30> interstage_locations;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(Config, interstage_locations);
    };

    /// Constructor using a the configuration provided in the input Data
    TruncateInterstageVariables();

    /// Destructor
    ~TruncateInterstageVariables() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_TRUNCATE_INTERSTAGE_VARIABLES_H_
