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

#ifndef SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_

#include "src/tint/lang/wgsl/sem/accessor_expression.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::ast {
class MemberAccessorExpression;
}  // namespace tint::ast
namespace tint::core::type {
class StructMember;
}  // namespace tint::core::type

namespace tint::sem {

/// StructMemberAccess holds the semantic information for a
/// ast::MemberAccessorExpression node that represents an access to a structure
/// member.
class StructMemberAccess final : public Castable<StructMemberAccess, AccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param object the object that holds the member being accessed
    /// @param member the structure member
    /// @param root_ident the (optional) root identifier for this expression
    StructMemberAccess(const ast::MemberAccessorExpression* declaration,
                       const core::type::Type* type,
                       const Statement* statement,
                       const core::constant::Value* constant,
                       const ValueExpression* object,
                       const core::type::StructMember* member,
                       const Variable* root_ident = nullptr);

    /// Destructor
    ~StructMemberAccess() override;

    /// @returns the structure member
    core::type::StructMember const* Member() const { return member_; }

  private:
    core::type::StructMember const* const member_;
};

/// Swizzle holds the semantic information for a ast::MemberAccessorExpression
/// node that represents a vector swizzle.
class Swizzle final : public Castable<Swizzle, AccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param object the object that holds the member being accessed
    /// @param indices the swizzle indices
    /// @param root_ident the (optional) root identifier for this expression
    Swizzle(const ast::MemberAccessorExpression* declaration,
            const core::type::Type* type,
            const Statement* statement,
            const core::constant::Value* constant,
            const ValueExpression* object,
            VectorRef<uint32_t> indices,
            const Variable* root_ident = nullptr);

    /// Destructor
    ~Swizzle() override;

    /// @return the swizzle indices, if this is a vector swizzle
    const auto& Indices() const { return indices_; }

  private:
    tint::Vector<uint32_t, 4> const indices_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
