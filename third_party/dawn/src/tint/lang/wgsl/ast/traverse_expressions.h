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

#ifndef SRC_TINT_LANG_WGSL_AST_TRAVERSE_EXPRESSIONS_H_
#define SRC_TINT_LANG_WGSL_AST_TRAVERSE_EXPRESSIONS_H_

#include <vector>

#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/literal_expression.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/phony_expression.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::ast {

/// The action to perform after calling the TraverseExpressions() callback
/// function.
enum class TraverseAction {
    /// Stop traversal immediately.
    Stop,
    /// Descend into this expression.
    Descend,
    /// Do not descend into this expression.
    Skip,
};

/// The order TraverseExpressions() will traverse expressions
enum class TraverseOrder {
    /// Expressions will be traversed from left to right
    LeftToRight,
    /// Expressions will be traversed from right to left
    RightToLeft,
};

/// TraverseExpressions performs a depth-first traversal of the expression nodes
/// from `root`, calling `callback` for each of the visited expressions that
/// match the predicate parameter type, in pre-ordering (root first).
/// @param root the root expression node
/// @param callback the callback function. Must be of the signature:
///        `TraverseAction(const T* expr)` or `TraverseAction(const T* expr, size_t depth)` where T
///        is an Expression type.
/// @return true on success, false on error
template <TraverseOrder ORDER = TraverseOrder::LeftToRight, typename CALLBACK>
bool TraverseExpressions(const Expression* root, CALLBACK&& callback) {
    using EXPR_TYPE = std::remove_pointer_t<traits::ParameterType<CALLBACK, 0>>;
    constexpr static bool kHasDepthArg = traits::SignatureOfT<CALLBACK>::parameter_count == 2;

    struct Pending {
        const Expression* expr;
        size_t depth;
    };

    tint::Vector<Pending, 32> to_visit{{root, 0}};

    auto push_single = [&](const Expression* expr, size_t depth) { to_visit.Push({expr, depth}); };
    auto push_pair = [&](const Expression* left, const Expression* right, size_t depth) {
        if constexpr (ORDER == TraverseOrder::LeftToRight) {
            to_visit.Push({right, depth});
            to_visit.Push({left, depth});
        } else {
            to_visit.Push({left, depth});
            to_visit.Push({right, depth});
        }
    };
    auto push_list = [&](VectorRef<const Expression*> exprs, size_t depth) {
        if constexpr (ORDER == TraverseOrder::LeftToRight) {
            for (auto* expr : tint::Reverse(exprs)) {
                to_visit.Push({expr, depth});
            }
        } else {
            for (auto* expr : exprs) {
                to_visit.Push({expr, depth});
            }
        }
    };

    while (!to_visit.IsEmpty()) {
        auto p = to_visit.Pop();
        const Expression* expr = p.expr;

        if (auto* filtered = expr->template As<EXPR_TYPE>()) {
            TraverseAction result;
            if constexpr (kHasDepthArg) {
                result = callback(filtered, p.depth);
            } else {
                result = callback(filtered);
            }

            switch (result) {
                case TraverseAction::Stop:
                    return true;
                case TraverseAction::Skip:
                    continue;
                case TraverseAction::Descend:
                    break;
            }
        }

        bool ok = Switch(
            expr,
            [&](const IdentifierExpression* ident) {
                if (auto* tmpl = ident->identifier->As<TemplatedIdentifier>()) {
                    push_list(tmpl->arguments, p.depth + 1);
                }
                return true;
            },
            [&](const IndexAccessorExpression* idx) {
                push_pair(idx->object, idx->index, p.depth + 1);
                return true;
            },
            [&](const BinaryExpression* bin_op) {
                push_pair(bin_op->lhs, bin_op->rhs, p.depth + 1);
                return true;
            },
            [&](const CallExpression* call) {
                if constexpr (ORDER == TraverseOrder::LeftToRight) {
                    push_list(call->args, p.depth + 1);
                    push_single(call->target, p.depth + 1);
                } else {
                    push_single(call->target, p.depth + 1);
                    push_list(call->args, p.depth + 1);
                }
                return true;
            },
            [&](const MemberAccessorExpression* member) {
                push_single(member->object, p.depth + 1);
                return true;
            },
            [&](const UnaryOpExpression* unary) {
                push_single(unary->expr, p.depth + 1);
                return true;
            },
            [&](const LiteralExpression*) { return true; },
            [&](const PhonyExpression*) { return true; },  //
            TINT_ICE_ON_NO_MATCH);
        if (!ok) {
            return false;
        }
    }
    return true;
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_TRAVERSE_EXPRESSIONS_H_
