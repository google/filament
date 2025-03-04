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

#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"

#include <unordered_set>
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/utils/containers/hashset.h"

#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/accessor_expression.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SimplifyPointers);

namespace tint::ast::transform {

namespace {

/// PointerOp describes either possible indirection or address-of action on an
/// expression.
struct PointerOp {
    /// Positive: Number of times the `expr` was dereferenced (*expr)
    /// Negative: Number of times the `expr` was 'addressed-of' (&expr)
    /// Zero: no pointer op on `expr`
    int indirections = 0;
    /// The expression being operated on
    const Expression* expr = nullptr;
};

}  // namespace

/// PIMPL state for the transform
struct SimplifyPointers::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// Set of accessor expression objects that are pointers, used to handle
    /// pointer-index/dot sugar syntax.
    Hashset<const Expression*, 4> is_accessor_object_pointer;

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Traverses the expression `expr` looking for non-literal array indexing
    /// expressions that would affect the computed address of a pointer
    /// expression. The function-like argument `cb` is called for each found.
    /// @param expr the expression to traverse
    /// @param cb a function-like object with the signature
    /// `void(const Expression*)`, which is called for each array index
    /// expression
    template <typename F>
    static void CollectSavedArrayIndices(const Expression* expr, F&& cb) {
        if (auto* a = expr->As<IndexAccessorExpression>()) {
            CollectSavedArrayIndices(a->object, cb);
            if (!a->index->Is<LiteralExpression>()) {
                cb(a->index);
            }
            return;
        }

        if (auto* m = expr->As<MemberAccessorExpression>()) {
            CollectSavedArrayIndices(m->object, cb);
            return;
        }

        if (auto* u = expr->As<UnaryOpExpression>()) {
            CollectSavedArrayIndices(u->expr, cb);
            return;
        }

        // Note: Other Expression types can be safely ignored as they cannot be
        // used to generate a reference or pointer.
        // See https://gpuweb.github.io/gpuweb/wgsl/#forming-references-and-pointers
    }

    /// Reduce walks the expression chain, collapsing all address-of and
    /// indirection ops into a PointerOp.
    /// @param in the expression to walk
    /// @returns the reduced PointerOp
    PointerOp Reduce(const Expression* in) {
        PointerOp op{0, in};
        while (true) {
            if (is_accessor_object_pointer.Contains(op.expr)) {
                // Object is an implicitly dereferenced pointer (i.e. syntax sugar).
                op.indirections++;
            }

            if (auto* unary = op.expr->As<UnaryOpExpression>()) {
                switch (unary->op) {
                    case core::UnaryOp::kIndirection:
                        op.indirections++;
                        op.expr = unary->expr;
                        continue;
                    case core::UnaryOp::kAddressOf:
                        op.indirections--;
                        op.expr = unary->expr;
                        continue;
                    default:
                        break;
                }
            }

            if (auto* sem = ctx.src->Sem().Get<sem::ValueExpression>(op.expr)) {
                // There may be an implicit load before the identifier due to a swizzle on pointer.
                if (auto* user = sem->UnwrapLoad()->As<sem::VariableUser>()) {
                    auto* var = user->Variable();
                    if (var->Is<sem::LocalVariable>() &&  //
                        var->Declaration()->Is<Let>() &&  //
                        var->Type()->Is<core::type::Pointer>()) {
                        op.expr = var->Declaration()->initializer;
                        continue;
                    }
                }
            }
            return op;
        }
    }

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        // A map of saved expressions to their saved variable name
        Hashmap<const Expression*, Symbol, 8> saved_vars;

        bool needs_transform = false;
        for (auto* ty : ctx.src->Types()) {
            if (ty->Is<core::type::Pointer>()) {
                // Program contains pointers which need removing.
                needs_transform = true;
                break;
            }
        }

        // Find all the pointer-typed `let` declarations.
        // Note that these must be function-scoped, as module-scoped `let`s are not
        // permitted.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            Switch(
                node,  //
                [&](const VariableDeclStatement* let) {
                    if (!let->variable->Is<Let>()) {
                        return;  // Not a `let` declaration. Ignore.
                    }

                    auto* var = ctx.src->Sem().Get(let->variable);
                    if (!var->Type()->Is<core::type::Pointer>()) {
                        return;  // Not a pointer type. Ignore.
                    }

                    // We're dealing with a pointer-typed `let` declaration.

                    // Scan the initializer expression for array index expressions that need
                    // to be hoist to temporary "saved" variables.
                    tint::Vector<const VariableDeclStatement*, 8> saved;
                    CollectSavedArrayIndices(
                        var->Declaration()->initializer, [&](const Expression* idx_expr) {
                            // We have a sub-expression that needs to be saved.
                            // Create a new variable
                            auto saved_name = ctx.dst->Symbols().New(
                                var->Declaration()->name->symbol.Name() + "_save");
                            auto* decl =
                                ctx.dst->Decl(ctx.dst->Let(saved_name, ctx.Clone(idx_expr)));
                            saved.Push(decl);
                            // Record the substitution of `idx_expr` to the saved variable
                            // with the symbol `saved_name`. This will be used by the
                            // ReplaceAll() handler above.
                            saved_vars.Add(idx_expr, saved_name);
                        });

                    // Find the place to insert the saved declarations.
                    // Special care needs to be made for lets declared as the initializer
                    // part of for-loops. In this case the block will hold the for-loop
                    // statement, not the let.
                    if (!saved.IsEmpty()) {
                        auto* stmt = ctx.src->Sem().Get(let);
                        auto* block = stmt->Block();
                        // Find the statement owned by the block (either the let decl or a
                        // for-loop)
                        while (block != stmt->Parent()) {
                            stmt = stmt->Parent();
                        }
                        // Declare the stored variables just before stmt. Order here is
                        // important as order-of-operations needs to be preserved.
                        // CollectSavedArrayIndices() visits the LHS of an index accessor
                        // before the index expression.
                        for (auto* decl : saved) {
                            // Note that repeated calls to InsertBefore() with the same `before`
                            // argument will result in nodes to inserted in the order the
                            // calls are made (last call is inserted last).
                            ctx.InsertBefore(block->Declaration()->statements, stmt->Declaration(),
                                             decl);
                        }
                    }

                    // As the original `let` declaration will be fully inlined, there's no
                    // need for the original declaration to exist. Remove it.
                    RemoveStatement(ctx, let);
                },
                [&](const UnaryOpExpression* op) {
                    if (op->op == core::UnaryOp::kAddressOf) {
                        // Transform can be skipped if no address-of operator is used, as there
                        // will be no pointers that can be inlined.
                        needs_transform = true;
                    }
                },
                [&](const AccessorExpression* accessor) {
                    if (auto* a = ctx.src->Sem().Get<sem::ValueExpression>(accessor->object)) {
                        // There may be an implicit load if this is a swizzle.
                        if (a->UnwrapLoad()->Type()->Is<core::type::Pointer>()) {
                            // Object is an implicitly dereferenced pointer (i.e. syntax sugar).
                            is_accessor_object_pointer.Add(accessor->object);
                        }
                    }
                });
        }

        if (!needs_transform) {
            return SkipTransform;
        }

        // Register the Expression transform handler.
        // This performs two different transformations:
        // * Identifiers that resolve to the pointer-typed `let` declarations are
        // replaced with the recursively inlined initializer expression for the
        // `let` declaration.
        // * Sub-expressions inside the pointer-typed `let` initializer expression
        // that have been hoisted to a saved variable are replaced with the saved
        // variable identifier.
        ctx.ReplaceAll([&](const Expression* expr) -> const Expression* {
            // Look to see if we need to swap this Expression with a saved variable.
            if (auto saved_var = saved_vars.Get(expr)) {
                return ctx.dst->Expr(*saved_var);
            }

            // Reduce the expression, folding away chains of address-of / indirections
            auto op = Reduce(expr);

            // Clone the reduced root expression
            expr = ctx.CloneWithoutTransform(op.expr);

            // And reapply the minimum number of address-of / indirections
            for (int i = 0; i < op.indirections; i++) {
                expr = ctx.dst->Deref(expr);
            }
            for (int i = 0; i > op.indirections; i--) {
                expr = ctx.dst->AddressOf(expr);
            }
            return expr;
        });

        ctx.Clone();
        return resolver::Resolve(b);
    }
};

SimplifyPointers::SimplifyPointers() = default;

SimplifyPointers::~SimplifyPointers() = default;

Transform::ApplyResult SimplifyPointers::Apply(const Program& src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::ast::transform
