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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::spirv::reader {

/// FoldTrivialLets is a transform that inlines the initializers of let declarations whose
/// initializers are just identifier expressions, or lets that are only used once. This is used to
/// clean up unnecessary let declarations created by the SPIR-V reader.
class FoldTrivialLets final : public Castable<FoldTrivialLets, ast::transform::Transform> {
  public:
    /// Constructor
    FoldTrivialLets();

    /// Destructor
    ~FoldTrivialLets() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_
