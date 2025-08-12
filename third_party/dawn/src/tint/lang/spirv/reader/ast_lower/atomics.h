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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_

#include <string>

#include "src/tint/lang/spirv/reader/ast_lower/transform.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/enums.h"

namespace tint::spirv::reader {

/// Atomics is a transform that replaces calls to stub functions created by the SPIR-V reader
/// with calls to the WGSL atomic builtin. It also makes sure to replace variable declarations that
/// are the target of the atomic operations with an atomic declaration of the same type. For
/// structs, it creates a copy of the original struct with atomic members.
class Atomics final : public Castable<Atomics, ast::transform::Transform> {
  public:
    /// Constructor
    Atomics();
    /// Destructor
    ~Atomics() override;

    /// Stub is an attribute applied to stub SPIR-V reader generated functions that need to be
    /// translated to an atomic builtin.
    class Stub final : public Castable<Stub, ast::InternalAttribute> {
      public:
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param builtin the atomic builtin this stub represents
        Stub(GenerationID pid, ast::NodeID nid, wgsl::BuiltinFn builtin);
        /// Destructor
        ~Stub() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Stub* Clone(ast::CloneContext& ctx) const override;

        /// The type of the intrinsic
        const wgsl::BuiltinFn builtin;
    };

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_
