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

#include "src/tint/lang/hlsl/writer/ast_raise/localize_struct_array_assignment.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/macros/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::LocalizeStructArrayAssignment);

namespace tint::hlsl::writer {

/// PIMPL state for the transform
struct LocalizeStructArrayAssignment::State {
    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        struct Shared {
            bool process_nested_nodes = false;
            Vector<const ast::Statement*, 4> insert_before_stmts;
            Vector<const ast::Statement*, 4> insert_after_stmts;
        } s;

        bool made_changes = false;

        for (auto* node : src.ASTNodes().Objects()) {
            if (auto* assign_stmt = node->As<ast::AssignmentStatement>()) {
                // Process if it's an assignment statement to a dynamically indexed array
                // within a struct on a function or private storage variable. This
                // specific use-case is what FXC fails to compile with:
                // error X3500: array reference cannot be used as an l-value; not natively
                // addressable
                if (!ContainsStructArrayIndex(assign_stmt->lhs)) {
                    continue;
                }
                auto og = GetOriginatingTypeAndAddressSpace(assign_stmt);
                if (!(og.first->Is<core::type::Struct>() &&
                      (og.second == core::AddressSpace::kFunction ||
                       og.second == core::AddressSpace::kPrivate))) {
                    continue;
                }

                ctx.Replace(assign_stmt, [&, assign_stmt] {
                    // Reset shared state for this assignment statement
                    s = Shared{};

                    const ast::Expression* new_lhs = nullptr;
                    {
                        TINT_SCOPED_ASSIGNMENT(s.process_nested_nodes, true);
                        new_lhs = ctx.Clone(assign_stmt->lhs);
                    }

                    auto* new_assign_stmt = b.Assign(new_lhs, ctx.Clone(assign_stmt->rhs));

                    // Combine insert_before_stmts + new_assign_stmt + insert_after_stmts into
                    // a block and return it
                    auto stmts = std::move(s.insert_before_stmts);
                    stmts.Reserve(1 + s.insert_after_stmts.Length());
                    stmts.Push(new_assign_stmt);
                    for (auto* stmt : s.insert_after_stmts) {
                        stmts.Push(stmt);
                    }

                    return b.Block(std::move(stmts));
                });

                made_changes = true;
            }
        }

        if (!made_changes) {
            return SkipTransform;
        }

        ctx.ReplaceAll(
            [&](const ast::IndexAccessorExpression* index_access) -> const ast::Expression* {
                if (!s.process_nested_nodes) {
                    return nullptr;
                }

                // Indexing a member access expr?
                auto* mem_access = index_access->object->As<ast::MemberAccessorExpression>();
                if (!mem_access) {
                    return nullptr;
                }

                // Process any nested IndexAccessorExpressions
                mem_access = ctx.Clone(mem_access);

                // Store the address of the member access into a let as we need to read
                // the value twice e.g. let tint_symbol = &(s.a1);
                auto mem_access_ptr = b.Sym();
                s.insert_before_stmts.Push(b.Decl(b.Let(mem_access_ptr, b.AddressOf(mem_access))));

                // Disable further transforms when cloning
                TINT_SCOPED_ASSIGNMENT(s.process_nested_nodes, false);

                // Copy entire array out of struct into local temp var
                // e.g. var tint_symbol_1 = *(tint_symbol);
                auto tmp_var = b.Sym();
                s.insert_before_stmts.Push(b.Decl(b.Var(tmp_var, b.Deref(mem_access_ptr))));

                // Replace input index_access with a clone of itself, but with its
                // .object replaced by the new temp var. This is returned from this
                // function to modify the original assignment statement. e.g.
                // tint_symbol_1[uniforms.i]
                auto* new_index_access = b.IndexAccessor(tmp_var, ctx.Clone(index_access->index));

                // Assign temp var back to array
                // e.g. *(tint_symbol) = tint_symbol_1;
                auto* assign_rhs_to_temp = b.Assign(b.Deref(mem_access_ptr), tmp_var);
                {
                    Vector<const ast::Statement*, 8> stmts{assign_rhs_to_temp};
                    for (auto* stmt : s.insert_after_stmts) {
                        stmts.Push(stmt);
                    }
                    s.insert_after_stmts = std::move(stmts);
                }

                return new_index_access;
            });

        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// Returns true if `expr` contains an index accessor expression to a
    /// structure member of array type.
    bool ContainsStructArrayIndex(const ast::Expression* expr) {
        bool result = false;
        TraverseExpressions(expr, [&](const ast::IndexAccessorExpression* ia) {
            // Indexing using a runtime value?
            auto* idx_sem = src.Sem().GetVal(ia->index);
            if (!idx_sem->ConstantValue()) {
                // Indexing a member access expr?
                if (auto* ma = ia->object->As<ast::MemberAccessorExpression>()) {
                    const auto* ma_ty = src.TypeOf(ma);
                    if (DAWN_UNLIKELY(ma_ty->Is<core::type::Pointer>())) {
                        TINT_ICE()
                            << "lhs of index accessor expression should not be a pointer. These "
                               "should have been removed by the SimplifyPointers transform";
                    }
                    // That accesses an array?
                    if (ma_ty->UnwrapRef()->Is<core::type::Array>()) {
                        result = true;
                        return ast::TraverseAction::Stop;
                    }
                }
            }
            return ast::TraverseAction::Descend;
        });

        return result;
    }

    // Returns the type and address space of the originating variable of the lhs
    // of the assignment statement.
    // See https://www.w3.org/TR/WGSL/#originating-variable-section
    std::pair<const core::type::Type*, core::AddressSpace> GetOriginatingTypeAndAddressSpace(
        const ast::AssignmentStatement* assign_stmt) {
        auto* root_ident = src.Sem().GetVal(assign_stmt->lhs)->RootIdentifier();
        if (DAWN_UNLIKELY(!root_ident)) {
            TINT_ICE() << "Unable to determine originating variable for lhs of assignment "
                          "statement";
        }

        return Switch(
            root_ident->Type(),  //
            [&](const core::type::Reference* ref) {
                return std::make_pair(ref->StoreType(), ref->AddressSpace());
            },
            [&](const core::type::Pointer* ptr) {
                return std::make_pair(ptr->StoreType(), ptr->AddressSpace());
            },  //
            TINT_ICE_ON_NO_MATCH);
    }
};

LocalizeStructArrayAssignment::LocalizeStructArrayAssignment() = default;

LocalizeStructArrayAssignment::~LocalizeStructArrayAssignment() = default;

ast::transform::Transform::ApplyResult LocalizeStructArrayAssignment::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    return State{src}.Run();
}

}  // namespace tint::hlsl::writer
