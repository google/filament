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

#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/transform.h"

namespace tint::ast::transform {

/// Private implementation of HoistToDeclBefore transform
struct HoistToDeclBefore::State {
    /// Constructor
    /// @param ctx_in the clone context
    explicit State(program::CloneContext& ctx_in) : ctx(ctx_in), b(*ctx_in.dst) {}

    /// @copydoc HoistToDeclBefore::Add()
    bool Add(const sem::ValueExpression* before_expr,
             const Expression* expr,
             VariableKind kind,
             const char* decl_name) {
        auto name = b.Symbols().New(decl_name);

        switch (kind) {
            case VariableKind::kLet: {
                auto* ty = ctx.src->Sem().GetVal(expr)->Type();
                TINT_ASSERT(!ty->IsAbstract());
                auto builder = [this, expr, name, ty] {
                    return b.Decl(b.Let(name, Transform::CreateASTTypeFor(ctx, ty),
                                        ctx.CloneWithoutTransform(expr)));
                };
                if (!InsertBeforeImpl(before_expr->Stmt(), std::move(builder))) {
                    return false;
                }
                break;
            }

            case VariableKind::kVar: {
                auto* ty = ctx.src->Sem().GetVal(expr)->Type();
                TINT_ASSERT(!ty->IsAbstract());
                auto builder = [this, expr, name, ty] {
                    return b.Decl(b.Var(name, Transform::CreateASTTypeFor(ctx, ty),
                                        ctx.CloneWithoutTransform(expr)));
                };
                if (!InsertBeforeImpl(before_expr->Stmt(), std::move(builder))) {
                    return false;
                }
                break;
            }

            case VariableKind::kConst: {
                auto builder = [this, expr, name] {
                    return b.Decl(b.Const(name, ctx.CloneWithoutTransform(expr)));
                };
                if (!InsertBeforeImpl(before_expr->Stmt(), std::move(builder))) {
                    return false;
                }
                break;
            }
        }

        // Replace the source expression with a reference to the hoisted declaration.
        ctx.Replace(expr, b.Expr(name));
        return true;
    }

    /// @copydoc HoistToDeclBefore::InsertBefore(const sem::Statement*, const Statement*)
    bool InsertBefore(const sem::Statement* before_stmt, const Statement* stmt) {
        if (stmt) {
            auto builder = [stmt] { return stmt; };
            return InsertBeforeImpl(before_stmt, std::move(builder));
        }
        return InsertBeforeImpl(before_stmt, Decompose{});
    }

    /// @copydoc HoistToDeclBefore::InsertBefore(const sem::Statement*, const StmtBuilder&)
    bool InsertBefore(const sem::Statement* before_stmt, const StmtBuilder& builder) {
        return InsertBeforeImpl(before_stmt, std::move(builder));
    }

    /// @copydoc HoistToDeclBefore::Replace(const sem::Statement* what, const Statement* with)
    bool Replace(const sem::Statement* what, const Statement* with) {
        auto builder = [with] { return with; };
        return Replace(what, std::move(builder));
    }

    /// @copydoc HoistToDeclBefore::Replace(const sem::Statement* what, const StmtBuilder& with)
    bool Replace(const sem::Statement* what, const StmtBuilder& with) {
        if (!InsertBeforeImpl(what, Decompose{})) {
            return false;
        }
        ctx.Replace(what->Declaration(), with);
        return true;
    }

    /// @copydoc HoistToDeclBefore::Prepare()
    bool Prepare(const sem::ValueExpression* before_expr) {
        return InsertBefore(before_expr->Stmt(), nullptr);
    }

  private:
    program::CloneContext& ctx;
    ast::Builder& b;

    /// Holds information about a for-loop that needs to be decomposed into a
    /// loop, so that declaration statements can be inserted before the
    /// condition expression or continuing statement.
    struct LoopInfo {
        Vector<StmtBuilder, 8> init_decls;
        Vector<StmtBuilder, 8> cond_decls;
        Vector<StmtBuilder, 8> cont_decls;
    };

    /// Info for each else-if that needs decomposing
    struct ElseIfInfo {
        /// Decls to insert before condition
        Vector<StmtBuilder, 8> cond_decls;
    };

    /// For-loops that need to be decomposed to loops.
    Hashmap<const sem::ForLoopStatement*, LoopInfo, 4> for_loops;

    /// Whiles that need to be decomposed to loops.
    Hashmap<const sem::WhileStatement*, LoopInfo, 4> while_loops;

    /// 'else if' statements that need to be decomposed to 'else {if}'
    Hashmap<const IfStatement*, ElseIfInfo, 4> else_ifs;

    template <size_t N>
    static auto Build(const Vector<StmtBuilder, N>& builders) {
        return tint::Transform(builders, [&](auto& builder) { return builder(); });
    }

    /// @returns a new LoopInfo reference for the given @p for_loop.
    /// @note if this is the first call to this method, then RegisterForLoopTransform() is
    /// automatically called.
    /// @warning the returned reference is invalid if this is called a second time, or the
    /// #for_loops map is mutated.
    LoopInfo& ForLoop(const sem::ForLoopStatement* for_loop) {
        if (for_loops.IsEmpty()) {
            RegisterForLoopTransform();
        }
        return for_loops.GetOrAddZero(for_loop);
    }

    /// @returns a new LoopInfo reference for the given @p while_loop.
    /// @note if this is the first call to this method, then RegisterWhileLoopTransform() is
    /// automatically called.
    /// @warning the returned reference is invalid if this is called a second time, or the
    /// #for_loops map is mutated.
    LoopInfo& WhileLoop(const sem::WhileStatement* while_loop) {
        if (while_loops.IsEmpty()) {
            RegisterWhileLoopTransform();
        }
        return while_loops.GetOrAddZero(while_loop);
    }

    /// @returns a new ElseIfInfo reference for the given @p else_if.
    /// @note if this is the first call to this method, then RegisterElseIfTransform() is
    /// automatically called.
    /// @warning the returned reference is invalid if this is called a second time, or the
    /// #else_ifs map is mutated.
    ElseIfInfo& ElseIf(const IfStatement* else_if) {
        if (else_ifs.IsEmpty()) {
            RegisterElseIfTransform();
        }
        return else_ifs.GetOrAddZero(else_if);
    }

    /// Registers the handler for transforming for-loops based on the content of the #for_loops map.
    void RegisterForLoopTransform() const {
        ctx.ReplaceAll([&](const ForLoopStatement* stmt) -> const Statement* {
            auto& sem = ctx.src->Sem();

            if (auto* fl = sem.Get(stmt)) {
                if (auto info = for_loops.Get(fl)) {
                    auto* for_loop = fl->Declaration();
                    // For-loop needs to be decomposed to a loop.
                    // Build the loop body's statements.
                    // Start with any let declarations for the conditional expression.
                    auto body_stmts = Build(info->cond_decls);
                    // If the for-loop has a condition, emit this next as:
                    //   if (!cond) { break; }
                    if (auto* cond = for_loop->condition) {
                        // !condition
                        auto* not_cond =
                            b.create<UnaryOpExpression>(core::UnaryOp::kNot, ctx.Clone(cond));
                        // { break; }
                        auto* break_body = b.Block(b.create<BreakStatement>());
                        // if (!condition) { break; }
                        body_stmts.Push(b.If(not_cond, break_body));
                    }
                    // Next emit the for-loop body
                    body_stmts.Push(ctx.Clone(for_loop->body));

                    // Create the continuing block if there was one.
                    const BlockStatement* continuing = nullptr;
                    if (auto* cont = for_loop->continuing) {
                        // Continuing block starts with any let declarations used by
                        // the continuing.
                        auto cont_stmts = Build(info->cont_decls);
                        cont_stmts.Push(ctx.Clone(cont));
                        continuing = b.Block(cont_stmts);
                    }

                    auto* body = b.Block(body_stmts);
                    auto* loop = b.Loop(body, continuing);

                    // If the loop has no initializer statements, then we're done.
                    // Otherwise, wrap loop with another block, prefixed with the initializer
                    // statements
                    if (!info->init_decls.IsEmpty() || for_loop->initializer) {
                        auto stmts = Build(info->init_decls);
                        if (auto* init = for_loop->initializer) {
                            stmts.Push(ctx.Clone(init));
                        }
                        stmts.Push(loop);
                        return b.Block(std::move(stmts));
                    }
                    return loop;
                }
            }
            return nullptr;
        });
    }

    /// Registers the handler for transforming while-loops based on the content of the #while_loops
    /// map.
    void RegisterWhileLoopTransform() const {
        // At least one while needs to be transformed into a loop.
        ctx.ReplaceAll([&](const WhileStatement* stmt) -> const Statement* {
            auto& sem = ctx.src->Sem();

            if (auto* w = sem.Get(stmt)) {
                if (auto info = while_loops.Get(w)) {
                    auto* while_loop = w->Declaration();
                    // While needs to be decomposed to a loop.
                    // Build the loop body's statements.
                    // Start with any let declarations for the conditional
                    // expression.
                    auto body_stmts =
                        tint::Transform(info->cond_decls, [&](auto& builder) { return builder(); });
                    // Emit the condition as:
                    //   if (!cond) { break; }
                    auto* cond = while_loop->condition;
                    // !condition
                    auto* not_cond = b.Not(ctx.Clone(cond));
                    // { break; }
                    auto* break_body = b.Block(b.Break());
                    // if (!condition) { break; }
                    body_stmts.Push(b.If(not_cond, break_body));

                    // Next emit the body
                    body_stmts.Push(ctx.Clone(while_loop->body));

                    const BlockStatement* continuing = nullptr;

                    auto* body = b.Block(body_stmts);
                    auto* loop = b.Loop(body, continuing);
                    return loop;
                }
            }
            return nullptr;
        });
    }

    /// Registers the handler for transforming if-statements based on the content of the #else_ifs
    /// map.
    void RegisterElseIfTransform() const {
        // Decompose 'else-if' statements into 'else { if }' blocks.
        ctx.ReplaceAll([&](const IfStatement* stmt) -> const Statement* {
            if (auto info = else_ifs.Get(stmt)) {
                // Build the else block's body statements, starting with let decls for the
                // conditional expression.
                auto body_stmts = Build(info->cond_decls);

                // Move the 'else-if' into the new `else` block as a plain 'if'.
                auto* cond = ctx.Clone(stmt->condition);
                auto* body = ctx.Clone(stmt->body);
                auto* new_if = b.If(cond, body, b.Else(ctx.Clone(stmt->else_statement)));
                body_stmts.Push(new_if);

                // Replace the 'else-if' with the new 'else' block.
                return b.Block(body_stmts);
            }
            return nullptr;
        });
    }

    /// A type used to signal to InsertBeforeImpl that no insertion should take place - instead flow
    /// control statements should just be decomposed.
    struct Decompose {};

    template <typename BUILDER>
    bool InsertBeforeImpl(const sem::Statement* before_stmt, BUILDER&& builder) {
        (void)builder;  // Avoid 'unused parameter' warning due to 'if constexpr'

        auto* ip = before_stmt->Declaration();

        auto* else_if = before_stmt->As<sem::IfStatement>();
        if (else_if && else_if->Parent()->Is<sem::IfStatement>()) {
            // Insertion point is an 'else if' condition.
            // Need to convert 'else if' to 'else { if }'.
            auto& else_if_info = ElseIf(else_if->Declaration());

            // Index the map to decompose this else if, even if `stmt` is nullptr.
            auto& decls = else_if_info.cond_decls;
            if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                decls.Push(std::forward<BUILDER>(builder));
            }
            return true;
        }

        if (auto* fl = before_stmt->As<sem::ForLoopStatement>()) {
            // Insertion point is a for-loop condition.
            // For-loop needs to be decomposed to a loop.

            // Index the map to decompose this for-loop, even if `stmt` is nullptr.
            auto& decls = ForLoop(fl).cond_decls;
            if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                decls.Push(std::forward<BUILDER>(builder));
            }
            return true;
        }

        if (auto* w = before_stmt->As<sem::WhileStatement>()) {
            // Insertion point is a while condition.
            // While needs to be decomposed to a loop.

            // Index the map to decompose this while, even if `stmt` is nullptr.
            auto& decls = WhileLoop(w).cond_decls;
            if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                decls.Push(std::forward<BUILDER>(builder));
            }
            return true;
        }

        auto* parent = before_stmt->Parent();  // The statement's parent
        if (auto* block = parent->As<sem::BlockStatement>()) {
            // Insert point sits in a block. Simple case.
            // Insert the stmt before the parent statement.
            if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                ctx.InsertBefore(block->Declaration()->statements, ip,
                                 std::forward<BUILDER>(builder));
            }
            return true;
        }

        auto* fl = parent->As<sem::ForLoopStatement>();
        if (DAWN_LIKELY(fl)) {
            // Insertion point is a for-loop initializer or continuing statement.
            // These require special care.
            if (fl->Declaration()->initializer == ip) {
                // Insertion point is a for-loop initializer.
                // For-loop needs to be decomposed to a loop.

                // Index the map to decompose this for-loop, even if `stmt` is nullptr.
                auto& decls = ForLoop(fl).init_decls;
                if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                    decls.Push(std::forward<BUILDER>(builder));
                }

                return true;
            }

            if (DAWN_LIKELY(fl->Declaration()->continuing == ip)) {
                // Insertion point is a for-loop continuing statement.
                // For-loop needs to be decomposed to a loop.

                // Index the map to decompose this for-loop, even if `stmt` is nullptr.
                auto& decls = ForLoop(fl).cont_decls;
                if constexpr (!std::is_same_v<BUILDER, Decompose>) {
                    decls.Push(std::forward<BUILDER>(builder));
                }

                return true;
            }

            TINT_ICE() << "unhandled use of expression in for-loop";
            return false;
        }

        TINT_ICE() << "unhandled expression parent statement type: " << parent->TypeInfo().name;
        return false;
    }
};

HoistToDeclBefore::HoistToDeclBefore(program::CloneContext& ctx)
    : state_(std::make_unique<State>(ctx)) {}

HoistToDeclBefore::~HoistToDeclBefore() {}

bool HoistToDeclBefore::Add(const sem::ValueExpression* before_expr,
                            const Expression* expr,
                            VariableKind kind,
                            const char* decl_name) {
    return state_->Add(before_expr, expr, kind, decl_name);
}

bool HoistToDeclBefore::InsertBefore(const sem::Statement* before_stmt, const Statement* stmt) {
    return state_->InsertBefore(before_stmt, stmt);
}

bool HoistToDeclBefore::InsertBefore(const sem::Statement* before_stmt,
                                     const StmtBuilder& builder) {
    return state_->InsertBefore(before_stmt, builder);
}

bool HoistToDeclBefore::Replace(const sem::Statement* what, const Statement* with) {
    return state_->Replace(what, with);
}

bool HoistToDeclBefore::Replace(const sem::Statement* what, const StmtBuilder& with) {
    return state_->Replace(what, with);
}

bool HoistToDeclBefore::Prepare(const sem::ValueExpression* before_expr) {
    return state_->Prepare(before_expr);
}

}  // namespace tint::ast::transform
