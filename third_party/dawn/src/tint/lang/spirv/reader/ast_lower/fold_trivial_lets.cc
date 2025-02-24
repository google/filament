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

#include "src/tint/lang/spirv/reader/ast_lower/fold_trivial_lets.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/hashmap.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::FoldTrivialLets);

namespace tint::spirv::reader {

/// PIMPL state for the transform.
struct FoldTrivialLets::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// The semantic info.
    const sem::Info& sem = src.Sem();

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Process a block.
    /// @param block the block
    void ProcessBlock(const ast::BlockStatement* block) {
        // PendingLet describes a let declaration that might be inlined.
        struct PendingLet {
            // The let declaration.
            const ast::VariableDeclStatement* decl = nullptr;
            // The number of uses that have not yet been inlined.
            size_t remaining_uses = 0;
        };

        // A map from semantic variables to their PendingLet descriptors.
        Hashmap<const sem::Variable*, PendingLet, 16> pending_lets;

        // Helper that folds pending let declarations into `expr` if possible.
        auto fold_lets = [&](const ast::Expression* expr) {
            ast::TraverseExpressions(expr, [&](const ast::IdentifierExpression* ident) {
                if (auto* user = sem.Get<sem::VariableUser>(ident)) {
                    if (auto itr = pending_lets.Get(user->Variable())) {
                        TINT_ASSERT(itr->remaining_uses > 0);

                        // We found a reference to a pending let, so replace it with the inlined
                        // initializer expression.
                        ctx.Replace(ident, ctx.Clone(itr->decl->variable->initializer));

                        // Decrement the remaining uses count and remove the let declaration if this
                        // was the last remaining use.
                        if (--(itr->remaining_uses) == 0) {
                            ctx.Remove(block->statements, itr->decl);
                        }
                    }
                }
                return ast::TraverseAction::Descend;
            });
        };

        // Loop over all statements in the block.
        for (auto* stmt : block->statements) {
            // Check for a let declarations.
            if (auto* decl = stmt->As<ast::VariableDeclStatement>()) {
                if (auto* let = decl->variable->As<ast::Let>()) {
                    // If the initializer doesn't have side effects, we might be able to inline it.
                    if (!sem.GetVal(let->initializer)->HasSideEffects()) {  //
                        auto num_users = sem.Get(let)->Users().Length();
                        if (let->initializer->Is<ast::IdentifierExpression>()) {
                            // The initializer is a single identifier expression.
                            // We can fold it into multiple uses in the next non-let statement.
                            // We also fold previous pending lets into this one, but only if
                            // it's only used once (to avoid duplicating potentially complex
                            // expressions).
                            if (num_users == 1) {
                                fold_lets(let->initializer);
                            }
                            pending_lets.Add(sem.Get(let), PendingLet{decl, num_users});
                        } else {
                            // The initializer is something more complex, so we only want to inline
                            // it if it's only used once.
                            // We also fold previous pending lets into this one.
                            fold_lets(let->initializer);
                            if (num_users == 1) {
                                pending_lets.Add(sem.Get(let), PendingLet{decl, 1});
                            }
                        }
                        continue;
                    }
                }
            }

            // Fold pending let declarations into a select few places that are frequently generated
            // by the SPIR_V reader.
            if (auto* assign = stmt->As<ast::AssignmentStatement>()) {
                // We can fold into the RHS of an assignment statement if the RHS and LHS
                // expressions have no side effects.
                if (!sem.GetVal(assign->lhs)->HasSideEffects() &&
                    !sem.GetVal(assign->rhs)->HasSideEffects()) {
                    fold_lets(assign->rhs);
                }
            } else if (auto* ifelse = stmt->As<ast::IfStatement>()) {
                // We can fold into the condition of an if statement if the condition expression has
                // no side effects.
                if (!sem.GetVal(ifelse->condition)->HasSideEffects()) {
                    fold_lets(ifelse->condition);
                }
            }

            // Clear any remaining pending lets.
            // We do not try to fold lets beyond the first non-let statement.
            pending_lets.Clear();
        }
    }

    /// Runs the transform.
    /// @returns the new program
    ApplyResult Run() {
        // Process all blocks in the module.
        for (auto* node : src.ASTNodes().Objects()) {
            if (auto* block = node->As<ast::BlockStatement>()) {
                ProcessBlock(block);
            }
        }
        ctx.Clone();
        return resolver::Resolve(b);
    }
};

FoldTrivialLets::FoldTrivialLets() = default;

FoldTrivialLets::~FoldTrivialLets() = default;

ast::transform::Transform::ApplyResult FoldTrivialLets::Apply(const Program& src,
                                                              const ast::transform::DataMap&,
                                                              ast::transform::DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::spirv::reader
