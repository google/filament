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

#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/get_insertion_point.h"
#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"
#include "src/tint/utils/macros/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::PromoteSideEffectsToDecl);

namespace tint::ast::transform {
namespace {

// Base state class for common members
class StateBase {
  protected:
    program::CloneContext& ctx;
    ast::Builder& b;
    const sem::Info& sem;

    explicit StateBase(program::CloneContext& ctx_in)
        : ctx(ctx_in), b(*ctx_in.dst), sem(ctx_in.src->Sem()) {}
};

// This first transform converts side-effecting for-loops to loops and else-ifs
// to else {if}s so that the next transform, DecomposeSideEffects, can insert
// hoisted expressions above their current location.
struct SimplifySideEffectStatements : Castable<PromoteSideEffectsToDecl, Transform> {
    ApplyResult Apply(const Program& src, const DataMap& inputs, DataMap& outputs) const override;
};

Transform::ApplyResult SimplifySideEffectStatements::Apply(const Program& src,
                                                           const DataMap&,
                                                           DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    bool made_changes = false;

    HoistToDeclBefore hoist_to_decl_before(ctx);
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* sem_expr = src.Sem().GetVal(node)) {
            if (!sem_expr->HasSideEffects()) {
                continue;
            }

            hoist_to_decl_before.Prepare(sem_expr);
            made_changes = true;
        }
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

// Decomposes side-effecting expressions to ensure order of evaluation. This
// handles both breaking down logical binary expressions for short-circuit
// evaluation, as well as hoisting expressions to ensure order of evaluation.
struct DecomposeSideEffects : Castable<PromoteSideEffectsToDecl, Transform> {
    class CollectHoistsState;
    class DecomposeState;
    ApplyResult Apply(const Program& src, const DataMap& inputs, DataMap& outputs) const override;
};

// CollectHoistsState traverses the AST top-down, identifying which expressions
// need to be hoisted to ensure order of evaluation, both those that give
// side-effects, as well as those that receive, and returns a set of these
// expressions.
using ToHoistSet = std::unordered_set<const Expression*>;
class DecomposeSideEffects::CollectHoistsState : public StateBase {
    // Expressions to hoist because they either cause or receive side-effects.
    ToHoistSet to_hoist;

    // Used to mark expressions as not or no longer having side-effects.
    std::unordered_set<const Expression*> no_side_effects;

    // Returns true if `expr` has side-effects. Unlike invoking
    // sem::ValueExpression::HasSideEffects(), this function takes into account whether
    // `expr` has been hoisted, returning false in that case. Furthermore, it
    // returns the correct result on parent expression nodes by traversing the
    // expression tree, memoizing the results to ensure O(1) amortized lookup.
    bool HasSideEffects(const Expression* expr) {
        if (no_side_effects.count(expr)) {
            return false;
        }

        return Switch(
            expr,  //
            [&](const CallExpression* e) -> bool { return sem.Get(e)->HasSideEffects(); },
            [&](const BinaryExpression* e) {
                if (HasSideEffects(e->lhs) || HasSideEffects(e->rhs)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const IndexAccessorExpression* e) {
                if (HasSideEffects(e->object) || HasSideEffects(e->index)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const MemberAccessorExpression* e) {
                if (HasSideEffects(e->object)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const UnaryOpExpression* e) {
                if (HasSideEffects(e->expr)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const IdentifierExpression* e) {
                no_side_effects.insert(e);
                return false;
            },
            [&](const LiteralExpression* e) {
                no_side_effects.insert(e);
                return false;
            },
            [&](const PhonyExpression* e) {
                no_side_effects.insert(e);
                return false;
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    // Adds `e` to `to_hoist` for hoisting to a let later on.
    void Hoist(const Expression* e) {
        no_side_effects.insert(e);
        to_hoist.emplace(e);
    }

    // Hoists any expressions in `maybe_hoist` and clears it
    template <size_t N>
    void Flush(tint::Vector<const Expression*, N>& maybe_hoist) {
        for (auto* m : maybe_hoist) {
            Hoist(m);
        }
        maybe_hoist.Clear();
    }

    // Recursive function that processes expressions for side-effects. It
    // traverses the expression tree child before parent, left-to-right. Each call
    // returns whether the input expression should maybe be hoisted, allowing the
    // parent node to decide whether to hoist or not. Generally:
    // * When 'true' is returned, the expression is added to the maybe_hoist list.
    // * When a side-effecting expression is met, we flush the expressions in the
    // maybe_hoist list, as they are potentially receivers of the side-effects.
    // * For index and member accessor expressions, special care is taken to not
    // over-hoist the lhs expressions, as these may be be chained to refer to a
    // single memory location.
    template <size_t N>
    bool ProcessExpression(const Expression* expr,
                           tint::Vector<const Expression*, N>& maybe_hoist) {
        auto process = [&](const Expression* e) -> bool {
            return ProcessExpression(e, maybe_hoist);
        };

        auto default_process = [&](const Expression* e) {
            auto maybe = process(e);
            if (maybe) {
                maybe_hoist.Push(e);
            }
            if (HasSideEffects(e)) {
                Flush(maybe_hoist);
            }
            return false;
        };

        auto binary_process = [&](const Expression* lhs, const Expression* rhs) {
            // If neither side causes side-effects, but at least one receives them,
            // let parent node hoist. This avoids over-hoisting side-effect receivers
            // of compound binary expressions (e.g. for "((a && b) && c) && f()", we
            // don't want to hoist each of "a", "b", and "c" separately, but want to
            // hoist "((a && b) && c)".
            if (!HasSideEffects(lhs) && !HasSideEffects(rhs)) {
                auto lhs_maybe = process(lhs);
                auto rhs_maybe = process(rhs);
                if (lhs_maybe || rhs_maybe) {
                    return true;
                }
                return false;
            }

            default_process(lhs);
            default_process(rhs);
            return false;
        };

        auto accessor_process = [&](const Expression* lhs, const Expression* rhs = nullptr) {
            auto maybe = process(lhs);
            // If lhs is a variable, let parent node hoist otherwise flush it right
            // away. This is to avoid over-hoisting the lhs of accessor chains (e.g.
            // for "v[a][b][c] + g()" we want to hoist all of "v[a][b][c]", not "t1 =
            // v[a]", then "t2 = t1[b]" then "t3 = t2[c]").
            if (maybe && HasSideEffects(lhs)) {
                maybe_hoist.Push(lhs);
                Flush(maybe_hoist);
                maybe = false;
            }
            if (rhs) {
                default_process(rhs);
            }
            return maybe;
        };

        return Switch(
            expr,
            [&](const CallExpression* e) -> bool {
                // We eagerly flush any variables in maybe_hoist for the current
                // call expression. Then we scope maybe_hoist to the processing of
                // the call args. This ensures that given: g(c, a(0), d) we hoist
                // 'c' because of 'a(0)', but not 'd' because there's no need, since
                // the call to g() will be hoisted if necessary.
                if (HasSideEffects(e)) {
                    Flush(maybe_hoist);
                }

                TINT_SCOPED_ASSIGNMENT(maybe_hoist, {});
                for (auto* a : e->args) {
                    default_process(a);
                }

                // Always hoist this call, even if it has no side-effects to ensure
                // left-to-right order of evaluation.
                // E.g. for "no_side_effects() + side_effects()", we want to hoist
                // no_side_effects() first.
                return true;
            },
            [&](const IdentifierExpression* e) {
                if (auto* sem_e = sem.GetVal(e)) {
                    if (auto* var_user = sem_e->UnwrapLoad()->As<sem::VariableUser>()) {
                        // Don't hoist constants.
                        if (var_user->ConstantValue()) {
                            return false;
                        }
                        // Don't hoist read-only variables as they cannot receive side-effects.
                        if (var_user->Variable()->Access() == core::Access::kRead) {
                            return false;
                        }
                        // Don't hoist textures / samplers as they can't be placed into a let, nor
                        // can they have side effects.
                        if (var_user->Variable()->Type()->IsHandle()) {
                            return false;
                        }
                        return true;
                    }
                }
                return false;
            },
            [&](const BinaryExpression* e) {
                if (e->IsLogical() && HasSideEffects(e)) {
                    // Don't hoist children of logical binary expressions with
                    // side-effects. These will be handled by DecomposeState.
                    process(e->lhs);
                    process(e->rhs);
                    return false;
                }
                return binary_process(e->lhs, e->rhs);
            },
            [&](const UnaryOpExpression* e) {  //
                auto r = process(e->expr);
                // Don't hoist address-of expressions.
                // E.g. for "g(&b, a(0))", we hoist "a(0)" only.
                if (e->op == core::UnaryOp::kAddressOf) {
                    return false;
                }
                return r;
            },
            [&](const IndexAccessorExpression* e) { return accessor_process(e->object, e->index); },
            [&](const MemberAccessorExpression* e) { return accessor_process(e->object); },
            [&](const LiteralExpression*) {
                // Leaf
                return false;
            },
            [&](const PhonyExpression*) {
                // Leaf
                return false;
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    // Starts the recursive processing of a statement's expression(s) to hoist side-effects to lets.
    void ProcessExpression(const Expression* expr) {
        if (!expr) {
            return;
        }

        tint::Vector<const Expression*, 8> maybe_hoist;
        ProcessExpression(expr, maybe_hoist);
    }

  public:
    explicit CollectHoistsState(program::CloneContext& ctx_in) : StateBase(ctx_in) {}

    ToHoistSet Run() {
        // Traverse all statements, recursively processing their expression tree(s)
        // to hoist side-effects to lets.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            auto* stmt = node->As<Statement>();
            if (!stmt) {
                continue;
            }

            Switch(
                stmt,  //
                [&](const AssignmentStatement* s) {
                    tint::Vector<const Expression*, 8> maybe_hoist;
                    ProcessExpression(s->lhs, maybe_hoist);
                    ProcessExpression(s->rhs, maybe_hoist);
                },
                [&](const CallStatement* s) {  //
                    ProcessExpression(s->expr);
                },
                [&](const ForLoopStatement* s) { ProcessExpression(s->condition); },
                [&](const WhileStatement* s) { ProcessExpression(s->condition); },
                [&](const IfStatement* s) {  //
                    ProcessExpression(s->condition);
                },
                [&](const ReturnStatement* s) {  //
                    ProcessExpression(s->value);
                },
                [&](const SwitchStatement* s) { ProcessExpression(s->condition); },
                [&](const VariableDeclStatement* s) {
                    ProcessExpression(s->variable->initializer);
                });
        }

        return std::move(to_hoist);
    }
};

// DecomposeState performs the actual transforming of the AST to ensure order of
// evaluation, using the set of expressions to hoist collected by
// CollectHoistsState.
class DecomposeSideEffects::DecomposeState : public StateBase {
    ToHoistSet to_hoist;

    // Returns true if `binary_expr` should be decomposed for short-circuit eval.
    bool IsLogicalWithSideEffects(const BinaryExpression* binary_expr) {
        return binary_expr->IsLogical() && (sem.GetVal(binary_expr->lhs)->HasSideEffects() ||
                                            sem.GetVal(binary_expr->rhs)->HasSideEffects());
    }

    // Recursive function used to decompose an expression for short-circuit eval.
    template <size_t N>
    const Expression* Decompose(const Expression* expr,
                                tint::Vector<const Statement*, N>* curr_stmts) {
        // Helper to avoid passing in same args.
        auto decompose = [&](auto& e) { return Decompose(e, curr_stmts); };

        // Clones `expr`, possibly hoisting it to a let.
        auto clone_maybe_hoisted = [&](const Expression* e) -> const Expression* {
            if (to_hoist.count(e)) {
                auto name = b.Symbols().New();
                auto* ty = sem.GetVal(e)->Type();
                auto* v = b.Let(name, Transform::CreateASTTypeFor(ctx, ty), ctx.Clone(e));
                auto* decl = b.Decl(v);
                curr_stmts->Push(decl);
                return b.Expr(name);
            }
            return ctx.Clone(e);
        };

        return Switch(
            expr,
            [&](const BinaryExpression* bin_expr) -> const Expression* {
                if (!IsLogicalWithSideEffects(bin_expr)) {
                    // No short-circuit, emit usual binary expr
                    ctx.Replace(bin_expr->lhs, decompose(bin_expr->lhs));
                    ctx.Replace(bin_expr->rhs, decompose(bin_expr->rhs));
                    return clone_maybe_hoisted(bin_expr);
                }

                // Decompose into ifs to implement short-circuiting
                // For example, 'let r = a && b' becomes:
                //
                // var temp = a;
                // if (temp) {
                //   temp = b;
                // }
                // let r = temp;
                //
                // and similarly, 'let r = a || b' becomes:
                //
                // var temp = a;
                // if (!temp) {
                //     temp = b;
                // }
                // let r = temp;
                //
                // Further, compound logical binary expressions are also handled
                // recursively, for example, 'let r = (a && (b && c))' becomes:
                //
                // var temp = a;
                // if (temp) {
                //     var temp2 = b;
                //     if (temp2) {
                //         temp2 = c;
                //     }
                //     temp = temp2;
                // }
                // let r = temp;

                auto name = b.Sym();
                curr_stmts->Push(b.Decl(b.Var(name, decompose(bin_expr->lhs))));

                const Expression* if_cond = nullptr;
                if (bin_expr->IsLogicalOr()) {
                    if_cond = b.Not(name);
                } else {
                    if_cond = b.Expr(name);
                }

                const BlockStatement* if_body = nullptr;
                {
                    tint::Vector<const Statement*, N> stmts;
                    TINT_SCOPED_ASSIGNMENT(curr_stmts, &stmts);
                    auto* new_rhs = decompose(bin_expr->rhs);
                    curr_stmts->Push(b.Assign(name, new_rhs));
                    if_body = b.Block(std::move(*curr_stmts));
                }

                curr_stmts->Push(b.If(if_cond, if_body));

                return b.Expr(name);
            },
            [&](const IndexAccessorExpression* idx) {
                ctx.Replace(idx->object, decompose(idx->object));
                ctx.Replace(idx->index, decompose(idx->index));
                return clone_maybe_hoisted(idx);
            },
            [&](const CallExpression* call) {
                for (auto* a : call->args) {
                    ctx.Replace(a, decompose(a));
                }
                return clone_maybe_hoisted(call);
            },
            [&](const MemberAccessorExpression* member) {
                ctx.Replace(member->object, decompose(member->object));
                return clone_maybe_hoisted(member);
            },
            [&](const UnaryOpExpression* unary) {
                ctx.Replace(unary->expr, decompose(unary->expr));
                return clone_maybe_hoisted(unary);
            },
            [&](const LiteralExpression* lit) {
                return clone_maybe_hoisted(lit);  // Leaf expression, just clone as is
            },
            [&](const IdentifierExpression* id) {
                return clone_maybe_hoisted(id);  // Leaf expression, just clone as is
            },
            [&](const PhonyExpression* phony) {
                return clone_maybe_hoisted(phony);  // Leaf expression, just clone as is
            },                                      //
            TINT_ICE_ON_NO_MATCH);
    }

    // Inserts statements in `stmts` before `stmt`
    template <size_t N>
    void InsertBefore(tint::Vector<const Statement*, N>& stmts, const Statement* stmt) {
        if (!stmts.IsEmpty()) {
            auto ip = utils::GetInsertionPoint(ctx, stmt);
            for (auto* s : stmts) {
                ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, s);
            }
        }
    }

    // Decomposes expressions of `stmt`, returning a replacement statement or
    // nullptr if not replacing it.
    const Statement* DecomposeStatement(const Statement* stmt) {
        return Switch(
            stmt,
            [&](const AssignmentStatement* s) -> const Statement* {
                if (!sem.GetVal(s->lhs)->HasSideEffects() &&
                    !sem.GetVal(s->rhs)->HasSideEffects()) {
                    return nullptr;
                }
                // lhs before rhs
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->lhs, Decompose(s->lhs, &stmts));
                ctx.Replace(s->rhs, Decompose(s->rhs, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const CallStatement* s) -> const Statement* {
                if (!sem.Get(s->expr)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->expr, Decompose(s->expr, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ForLoopStatement* s) -> const Statement* {
                if (!s->condition || !sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const WhileStatement* s) -> const Statement* {
                if (!sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const IfStatement* s) -> const Statement* {
                if (!sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ReturnStatement* s) -> const Statement* {
                if (!s->value || !sem.GetVal(s->value)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->value, Decompose(s->value, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const SwitchStatement* s) -> const Statement* {
                if (!sem.Get(s->condition)) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const VariableDeclStatement* s) -> const Statement* {
                auto* var = s->variable;
                if (!var->initializer || !sem.GetVal(var->initializer)->HasSideEffects()) {
                    return nullptr;
                }
                tint::Vector<const Statement*, 8> stmts;
                ctx.Replace(var->initializer, Decompose(var->initializer, &stmts));
                InsertBefore(stmts, s);
                return b.Decl(ctx.CloneWithoutTransform(var));
            },
            [](Default) -> const Statement* {
                // Other statement types don't have expressions
                return nullptr;
            });
    }

  public:
    explicit DecomposeState(program::CloneContext& ctx_in, ToHoistSet to_hoist_in)
        : StateBase(ctx_in), to_hoist(std::move(to_hoist_in)) {}

    void Run() {
        // We replace all BlockStatements as this allows us to iterate over the
        // block statements and ctx.InsertBefore hoisted declarations on them.
        ctx.ReplaceAll([&](const BlockStatement* block) -> const Statement* {
            for (auto* stmt : block->statements) {
                if (auto* new_stmt = DecomposeStatement(stmt)) {
                    ctx.Replace(stmt, new_stmt);
                }

                // Handle for loops, as they are the only other AST node that
                // contains statements outside of BlockStatements.
                if (auto* fl = stmt->As<ForLoopStatement>()) {
                    if (auto* new_stmt = DecomposeStatement(fl->initializer)) {
                        ctx.Replace(fl->initializer, new_stmt);
                    }
                    if (auto* new_stmt = DecomposeStatement(fl->continuing)) {
                        ctx.Replace(fl->continuing, new_stmt);
                    }
                }
            }
            return nullptr;
        });
    }
};

Transform::ApplyResult DecomposeSideEffects::Apply(const Program& src,
                                                   const DataMap&,
                                                   DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    // First collect side-effecting expressions to hoist
    CollectHoistsState collect_hoists_state{ctx};
    auto to_hoist = collect_hoists_state.Run();

    // Now decompose these expressions
    DecomposeState decompose_state{ctx, std::move(to_hoist)};
    decompose_state.Run();

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace

PromoteSideEffectsToDecl::PromoteSideEffectsToDecl() = default;
PromoteSideEffectsToDecl::~PromoteSideEffectsToDecl() = default;

Transform::ApplyResult PromoteSideEffectsToDecl::Apply(const Program& src,
                                                       const DataMap& inputs,
                                                       DataMap& outputs) const {
    Manager manager;
    manager.Add<SimplifySideEffectStatements>();
    manager.Add<DecomposeSideEffects>();
    return manager.Run(src, inputs, outputs);
}

}  // namespace tint::ast::transform
