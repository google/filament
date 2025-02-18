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

#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"

#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/utils/containers/hashset.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::PromoteInitializersToLet);

namespace tint::ast::transform {

PromoteInitializersToLet::PromoteInitializersToLet() = default;

PromoteInitializersToLet::~PromoteInitializersToLet() = default;

Transform::ApplyResult PromoteInitializersToLet::Apply(const Program& src,
                                                       const DataMap&,
                                                       DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    // Returns true if the expression should be hoisted to a new let statement before the
    // expression's statement.
    auto should_hoist = [&](const sem::ValueExpression* expr) {
        if (!expr->Type()->IsAnyOf<core::type::Array, core::type::Struct>()) {
            // We only care about array and struct initializers
            return false;
        }

        if (expr->Type()->IsAbstract()) {
            // Do not hoist expressions that are not materialized, as doing so would cause
            // premature materialization.
            return false;
        }

        // Check whether the expression is an array or structure constructor
        {
            // Follow const-chains
            auto* root_expr = expr;
            if (expr->Stage() == core::EvaluationStage::kConstant) {
                while (auto* user = root_expr->UnwrapMaterialize()->As<sem::VariableUser>()) {
                    root_expr = user->Variable()->Initializer();
                }
            }

            auto* ctor = root_expr->UnwrapMaterialize()->As<sem::Call>();
            if (!ctor || !ctor->Target()->Is<sem::ValueConstructor>()) {
                // Root expression is not a value constructor. Not interested in this.
                return false;
            }
        }

        if (auto* src_var_decl = expr->Stmt()->Declaration()->As<VariableDeclStatement>()) {
            if (src_var_decl->variable->initializer == expr->Declaration()) {
                // This statement is just a variable declaration with the initializer as the
                // initializer value. This is what we're attempting to transform to, and so
                // ignore.
                return false;
            }
        }

        return true;
    };

    // A list of expressions that should be hoisted.
    tint::Vector<const sem::ValueExpression*, 32> to_hoist;
    // A set of expressions that are constant, which _may_ need to be hoisted.
    Hashset<const Expression*, 32> const_chains;

    // Walk the AST nodes. This order guarantees that leaf-expressions are visited first.
    for (auto* node : src.ASTNodes().Objects()) {
        if (auto* sem = src.Sem().GetVal(node)) {
            auto* stmt = sem->Stmt();
            if (!stmt) {
                // Expression is outside of a statement. This usually means the expression is part
                // of a global (module-scope) constant declaration. These must be constexpr, and so
                // cannot contain the type of expressions that must be sanitized.
                continue;
            }

            if (sem->Stage() == core::EvaluationStage::kConstant ||
                sem->Stage() == core::EvaluationStage::kNotEvaluated) {
                // Expression is constant or not evaluated. We only need to hoist expressions if
                // they're the outermost constant expression in a chain. Remove the immediate child
                // nodes of the expression from const_chains, and add this expression to the
                // const_chains. As we visit leaf-expressions first, this means the content of
                // const_chains only contains the outer-most constant expressions.
                auto* expr = sem->Declaration();
                bool ok = TraverseExpressions(expr, [&](const Expression* child) {
                    const_chains.Remove(child);
                    return child == expr ? TraverseAction::Descend : TraverseAction::Skip;
                });
                if (!ok) {
                    return resolver::Resolve(b);
                }
                if (sem->Stage() == core::EvaluationStage::kConstant) {
                    const_chains.Add(expr);
                }
            } else if (should_hoist(sem)) {
                to_hoist.Push(sem);
            }
        }
    }

    // After walking the full AST, const_chains only contains the outer-most constant expressions.
    // Check if any of these need hoisting, and append those to to_hoist.
    for (auto& expr : const_chains) {
        if (auto* sem = src.Sem().GetVal(expr.Value()); should_hoist(sem)) {
            to_hoist.Push(sem);
        }
    }

    if (to_hoist.IsEmpty()) {
        // Nothing to do. Skip.
        return SkipTransform;
    }

    // The order of to_hoist is currently undefined. Sort by AST node id, which will make this
    // deterministic.
    to_hoist.Sort([&](auto* expr_a, auto* expr_b) {
        return expr_a->Declaration()->node_id < expr_b->Declaration()->node_id;
    });

    // Hoist all the expressions in to_hoist to a constant variable, declared just before the
    // statement of usage.
    HoistToDeclBefore hoist_to_decl_before(ctx);
    for (auto* expr : to_hoist) {
        if (!hoist_to_decl_before.Add(expr, expr->Declaration(),
                                      HoistToDeclBefore::VariableKind::kLet)) {
            return resolver::Resolve(b);
        }
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
