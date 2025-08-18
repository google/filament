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

#ifndef SRC_TINT_LANG_WGSL_AST_BINARY_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_AST_BINARY_EXPRESSION_H_

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/wgsl/ast/expression.h"

namespace tint::ast {

/// An binary expression
class BinaryExpression final : public Castable<BinaryExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the binary expression source
    /// @param op the operation type
    /// @param lhs the left side of the expression
    /// @param rhs the right side of the expression
    BinaryExpression(GenerationID pid,
                     NodeID nid,
                     const Source& source,
                     core::BinaryOp op,
                     const Expression* lhs,
                     const Expression* rhs);
    /// Move constructor
    BinaryExpression(BinaryExpression&&);
    ~BinaryExpression() override;

    /// @returns true if the op is and
    bool IsAnd() const { return op == core::BinaryOp::kAnd; }
    /// @returns true if the op is or
    bool IsOr() const { return op == core::BinaryOp::kOr; }
    /// @returns true if the op is xor
    bool IsXor() const { return op == core::BinaryOp::kXor; }
    /// @returns true if the op is logical and
    bool IsLogicalAnd() const { return op == core::BinaryOp::kLogicalAnd; }
    /// @returns true if the op is logical or
    bool IsLogicalOr() const { return op == core::BinaryOp::kLogicalOr; }
    /// @returns true if the op is equal
    bool IsEqual() const { return op == core::BinaryOp::kEqual; }
    /// @returns true if the op is not equal
    bool IsNotEqual() const { return op == core::BinaryOp::kNotEqual; }
    /// @returns true if the op is less than
    bool IsLessThan() const { return op == core::BinaryOp::kLessThan; }
    /// @returns true if the op is greater than
    bool IsGreaterThan() const { return op == core::BinaryOp::kGreaterThan; }
    /// @returns true if the op is less than equal
    bool IsLessThanEqual() const { return op == core::BinaryOp::kLessThanEqual; }
    /// @returns true if the op is greater than equal
    bool IsGreaterThanEqual() const { return op == core::BinaryOp::kGreaterThanEqual; }
    /// @returns true if the op is shift left
    bool IsShiftLeft() const { return op == core::BinaryOp::kShiftLeft; }
    /// @returns true if the op is shift right
    bool IsShiftRight() const { return op == core::BinaryOp::kShiftRight; }
    /// @returns true if the op is add
    bool IsAdd() const { return op == core::BinaryOp::kAdd; }
    /// @returns true if the op is subtract
    bool IsSubtract() const { return op == core::BinaryOp::kSubtract; }
    /// @returns true if the op is multiply
    bool IsMultiply() const { return op == core::BinaryOp::kMultiply; }
    /// @returns true if the op is divide
    bool IsDivide() const { return op == core::BinaryOp::kDivide; }
    /// @returns true if the op is modulo
    bool IsModulo() const { return op == core::BinaryOp::kModulo; }
    /// @returns true if the op is an arithmetic operation
    bool IsArithmetic() const;
    /// @returns true if the op is a comparison operation
    bool IsComparison() const;
    /// @returns true if the op is a bitwise operation
    bool IsBitwise() const;
    /// @returns true if the op is a bit shift operation
    bool IsBitshift() const;
    /// @returns true if the op is a logical expression
    bool IsLogical() const;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const BinaryExpression* Clone(CloneContext& ctx) const override;

    /// the binary op type
    const core::BinaryOp op;
    /// the left side expression
    const Expression* const lhs;
    /// the right side expression
    const Expression* const rhs;
};

/// @param op the operator
/// @returns true if the op is an arithmetic operation
inline bool IsArithmetic(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
        case core::BinaryOp::kMultiply:
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a comparison operation
inline bool IsComparison(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
        case core::BinaryOp::kLessThan:
        case core::BinaryOp::kLessThanEqual:
        case core::BinaryOp::kGreaterThan:
        case core::BinaryOp::kGreaterThanEqual:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a bitwise operation
inline bool IsBitwise(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a bit shift operation
inline bool IsBitshift(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return true;
        default:
            return false;
    }
}

inline bool BinaryExpression::IsLogical() const {
    switch (op) {
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr:
            return true;
        default:
            return false;
    }
}

inline bool BinaryExpression::IsArithmetic() const {
    return ast::IsArithmetic(op);
}

inline bool BinaryExpression::IsComparison() const {
    return ast::IsComparison(op);
}

inline bool BinaryExpression::IsBitwise() const {
    return ast::IsBitwise(op);
}

inline bool BinaryExpression::IsBitshift() const {
    return ast::IsBitshift(op);
}

/// @returns the human readable name of the given core::BinaryOp
/// @param op the core::BinaryOp
constexpr const char* FriendlyName(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
            return "and";
        case core::BinaryOp::kOr:
            return "or";
        case core::BinaryOp::kXor:
            return "xor";
        case core::BinaryOp::kLogicalAnd:
            return "logical_and";
        case core::BinaryOp::kLogicalOr:
            return "logical_or";
        case core::BinaryOp::kEqual:
            return "equal";
        case core::BinaryOp::kNotEqual:
            return "not_equal";
        case core::BinaryOp::kLessThan:
            return "less_than";
        case core::BinaryOp::kGreaterThan:
            return "greater_than";
        case core::BinaryOp::kLessThanEqual:
            return "less_than_equal";
        case core::BinaryOp::kGreaterThanEqual:
            return "greater_than_equal";
        case core::BinaryOp::kShiftLeft:
            return "shift_left";
        case core::BinaryOp::kShiftRight:
            return "shift_right";
        case core::BinaryOp::kAdd:
            return "add";
        case core::BinaryOp::kSubtract:
            return "subtract";
        case core::BinaryOp::kMultiply:
            return "multiply";
        case core::BinaryOp::kDivide:
            return "divide";
        case core::BinaryOp::kModulo:
            return "modulo";
    }
    return "<invalid>";
}

/// @returns the WGSL operator of the core::BinaryOp
/// @param op the core::BinaryOp
constexpr const char* Operator(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
            return "&";
        case core::BinaryOp::kOr:
            return "|";
        case core::BinaryOp::kXor:
            return "^";
        case core::BinaryOp::kLogicalAnd:
            return "&&";
        case core::BinaryOp::kLogicalOr:
            return "||";
        case core::BinaryOp::kEqual:
            return "==";
        case core::BinaryOp::kNotEqual:
            return "!=";
        case core::BinaryOp::kLessThan:
            return "<";
        case core::BinaryOp::kGreaterThan:
            return ">";
        case core::BinaryOp::kLessThanEqual:
            return "<=";
        case core::BinaryOp::kGreaterThanEqual:
            return ">=";
        case core::BinaryOp::kShiftLeft:
            return "<<";
        case core::BinaryOp::kShiftRight:
            return ">>";
        case core::BinaryOp::kAdd:
            return "+";
        case core::BinaryOp::kSubtract:
            return "-";
        case core::BinaryOp::kMultiply:
            return "*";
        case core::BinaryOp::kDivide:
            return "/";
        case core::BinaryOp::kModulo:
            return "%";
    }
    return "<invalid>";
}

/// @param out the stream to write to
/// @param op the core::BinaryOp
/// @return the stream so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, core::BinaryOp op) {
    out << FriendlyName(op);
    return out;
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_BINARY_EXPRESSION_H_
