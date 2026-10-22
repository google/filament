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

#ifndef SRC_TINT_LANG_WGSL_AST_CALL_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_AST_CALL_EXPRESSION_H_

#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// A call expression - represents either a:
/// * sem::Function
/// * sem::BuiltinFn
/// * sem::ValueConstructor
/// * sem::ValueConversion
class CallExpression final : public Castable<CallExpression, Expression> {
  public:
    /// Constructor
    /// @param nid the unique node identifier
    /// @param source the call expression source
    /// @param target the target of the call
    /// @param args the arguments
    CallExpression(NodeID nid,
                   const Source& source,
                   const IdentifierExpression* target,
                   VectorRef<const Expression*> args);

    /// Destructor
    ~CallExpression() override;

    /// The target function or type
    const IdentifierExpression* target;

    /// The arguments
    const tint::Vector<const Expression*, 8> args;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_CALL_EXPRESSION_H_
