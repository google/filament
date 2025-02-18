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

#ifndef SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_
#define SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_

#include <string>

#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::hlsl::writer {

/// PixelLocal transforms module-scope `var<pixel_local>`s and fragment entry point functions that
/// use them:
/// * `var<pixel_local>` will be transformed to `var<private>`, a bunch of read-write storage
///   textures with a special `RasterizerOrderedView` attribute, a function to read the data from
///   thes read-write storage textures into the `var<private>` (ROV Reader), and a function to write
///   the data from the `var<private>` into the read-write storage textures (ROV Writer).
/// * The entry point function will be wrapped with another function ('outer') that calls the
///  'inner' function.
/// * The outer function will always have `@builtin(position)` in its input parameters.
/// * The outer function will call the ROV reader before the original entry point function, and call
///   the ROV writer after the original entry point function.
/// @note PixelLocal requires that the SingleEntryPoint transform has already been run
/// TODO(tint:2083): Optimize this in the future by inserting the load as late as possible and the
/// store as early as possible.
class PixelLocal final : public Castable<PixelLocal, ast::transform::Transform> {
  public:
    /// Transform configuration options
    struct Config final : public Castable<Config, ast::transform::Data> {
        /// Constructor
        Config();

        /// Copy Constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Index of pixel_local structure member index to ROV register
        Hashmap<uint32_t, uint32_t, 8> pls_member_to_rov_reg;

        /// Index of pixel_local structure member index to ROV format
        Hashmap<uint32_t, core::TexelFormat, 8> pls_member_to_rov_format;

        /// Group index of all ROVs
        uint32_t rov_group_index = 0;
    };

    /// RasterizerOrderedView is an InternalAttribute that's used to decorate a read-write storage
    /// texture object.
    class RasterizerOrderedView final
        : public Castable<RasterizerOrderedView, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        RasterizerOrderedView(GenerationID pid, ast::NodeID nid);

        /// Destructor
        ~RasterizerOrderedView() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const RasterizerOrderedView* Clone(ast::CloneContext& ctx) const override;
    };

    /// Constructor
    PixelLocal();

    /// Destructor
    ~PixelLocal() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_
