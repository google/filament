// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_VERTEX_PULLING_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_VERTEX_PULLING_H_

#include <vector>

#include "src/tint/api/common/vertex_pulling_config.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// Converts a program to use vertex pulling
///
/// Variables which accept vertex input are var<in> with a location attribute.
/// This transform will convert those to be assigned from storage buffers
/// instead. The intention is to allow vertex input to rely on a storage buffer
/// clamping pass for out of bounds reads. We bind the storage buffers as arrays
/// of u32, so any read to byte position `p` will actually need to read position
/// `p / 4`, since `sizeof(u32) == 4`.
///
/// `VertexFormat` represents the input type of the attribute. This isn't
/// related to the type of the variable in the shader. For example,
/// `VertexFormat::kVec2F16` tells us that the buffer will contain `f16`
/// elements, to be read as vec2. In the shader, a user would make a `vec2<f32>`
/// to be able to use them. The conversion between `f16` and `f32` will need to
/// be handled by us (using unpack functions).
///
/// To be clear, there won't be types such as `f16` or `u8` anywhere in WGSL
/// code, but these are types that the data may arrive as. We need to convert
/// these smaller types into the base types such as `f32` and `u32` for the
/// shader to use.
///
/// The SingleEntryPoint transform must have run before VertexPulling.
class VertexPulling final : public Castable<VertexPulling, Transform> {
  public:
    /// Configuration options for the transform
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// The vertex state descriptor, containing info about attributes
        std::vector<VertexBufferLayoutDescriptor> vertex_state;

        /// The "group" we will put all our vertex buffers into (as storage buffers)
        /// Default to 4 as it is past the limits of user-accessible groups
        uint32_t pulling_group = 4u;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(Config, vertex_state, pulling_group);
    };

    /// Constructor
    VertexPulling();

    /// Destructor
    ~VertexPulling() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;

    Config cfg_;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_VERTEX_PULLING_H_
