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

#ifndef SRC_TINT_LANG_WGSL_SEM_LOAD_H_
#define SRC_TINT_LANG_WGSL_SEM_LOAD_H_

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"

namespace tint::sem {

/// Load is a semantic expression which represents the load of a memory view to a value.
/// Loads from reference types are implicit in WGSL, so the Load semantic node shares the same AST
/// node as the inner semantic node.
class Load final : public Castable<Load, ValueExpression> {
  public:
    /// Constructor
    /// @param source the source expression being loaded from
    /// @param statement the statement that owns this expression
    /// @param stage the earliest evaluation stage for the expression
    Load(const ValueExpression* source, const Statement* statement, core::EvaluationStage stage);

    /// Destructor
    ~Load() override;

    /// @return the source object being loaded
    const ValueExpression* Source() const { return source_; }

    /// @returns the type of the memory view being loaded from.
    const core::type::MemoryView* MemoryView() const {
        return static_cast<const core::type::MemoryView*>(source_->Type());
    }

  private:
    ValueExpression const* const source_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_LOAD_H_
