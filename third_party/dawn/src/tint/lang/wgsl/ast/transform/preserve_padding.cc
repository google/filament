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

#include "src/tint/lang/wgsl/ast/transform/preserve_padding.h"

#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::PreservePadding);

namespace tint::ast::transform {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

PreservePadding::PreservePadding() = default;

PreservePadding::~PreservePadding() = default;

/// The PIMPL state for the PreservePadding transform
struct PreservePadding::State {
    /// Constructor
    /// @param src the source Program
    explicit State(const Program& src) : ctx{&b, &src, /* auto_clone_symbols */ true} {}

    /// The main function for the transform.
    /// @returns the ApplyResult
    ApplyResult Run() {
        // Gather a list of assignments that need to be transformed.
        std::unordered_set<const AssignmentStatement*> assignments_to_transform;
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            Switch(node,  //
                   [&](const AssignmentStatement* assign) {
                       auto* ty = sem.GetVal(assign->lhs)->Type();
                       if (assign->lhs->Is<PhonyExpression>()) {
                           // Ignore phony assignment.
                           return;
                       }
                       if (ty->As<core::type::Reference>()->AddressSpace() !=
                           core::AddressSpace::kStorage) {
                           // We only care about assignments that write to variables in the storage
                           // address space, as nothing else is host-visible.
                           return;
                       }
                       if (HasPadding(ty->UnwrapRef())) {
                           // The assigned type has padding bytes, so we need to decompose the
                           // writes.
                           assignments_to_transform.insert(assign);
                       }
                   });
        }
        if (assignments_to_transform.empty()) {
            return SkipTransform;
        }

        // Replace all assignments that include padding with decomposed versions.
        ctx.ReplaceAll([&](const AssignmentStatement* assign) -> const Statement* {
            if (!assignments_to_transform.count(assign)) {
                return nullptr;
            }
            auto* ty = sem.GetVal(assign->lhs)->Type()->UnwrapRef();
            return MakeAssignment(ty, ctx.Clone(assign->lhs), ctx.Clone(assign->rhs));
        });

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Create a statement that will perform the assignment `lhs = rhs`, creating and using helper
    /// functions to decompose the assignment into element-wise copies if needed.
    /// @param ty the type of the assignment
    /// @param lhs the lhs expression (in the destination program)
    /// @param rhs the rhs expression (in the destination program)
    /// @returns the statement that performs the assignment
    const Statement* MakeAssignment(const core::type::Type* ty,
                                    const Expression* lhs,
                                    const Expression* rhs) {
        if (!HasPadding(ty)) {
            // No padding - use a regular assignment.
            return b.Assign(lhs, rhs);
        }

        // Call (and create if necessary) a helper function that assigns a composite using the
        // statements in `body`. The helper will have the form:
        //   fn assign_helper_T(dest : ptr<storage, T, read_write>, value : T) {
        //     <body>
        //   }
        // It will be called by passing a pointer to the original LHS:
        //   assign_helper_T(&lhs, rhs);
        const char* kDestParamName = "dest";
        const char* kValueParamName = "value";
        auto call_helper = [&](auto&& body) {
            auto helper = helpers.GetOrAdd(ty, [&] {
                auto helper_name = b.Symbols().New("assign_and_preserve_padding");
                tint::Vector<const Parameter*, 2> params = {
                    b.Param(kDestParamName,
                            b.ty.ptr<storage, read_write>(CreateASTTypeFor(ctx, ty))),
                    b.Param(kValueParamName, CreateASTTypeFor(ctx, ty)),
                };
                b.Func(helper_name, params, b.ty.void_(), body());
                return helper_name;
            });
            return b.CallStmt(b.Call(helper, b.AddressOf(lhs), rhs));
        };

        return Switch(
            ty,  //
            [&](const core::type::Array* arr) {
                // Call a helper function that uses a loop to assigns each element separately.
                return call_helper([&] {
                    tint::Vector<const Statement*, 8> body;
                    auto* idx = b.Var("i", b.Expr(0_u));
                    body.Push(
                        b.For(b.Decl(idx), b.LessThan(idx, u32(arr->ConstantCount().value())),
                              b.Assign(idx, b.Add(idx, 1_u)),
                              b.Block(MakeAssignment(arr->ElemType(),
                                                     b.IndexAccessor(b.Deref(kDestParamName), idx),
                                                     b.IndexAccessor(kValueParamName, idx)))));
                    return body;
                });
            },
            [&](const core::type::Matrix* mat) {
                // Call a helper function that assigns each column separately.
                return call_helper([&] {
                    tint::Vector<const Statement*, 4> body;
                    for (uint32_t i = 0; i < mat->Columns(); i++) {
                        body.Push(MakeAssignment(mat->ColumnType(),
                                                 b.IndexAccessor(b.Deref(kDestParamName), u32(i)),
                                                 b.IndexAccessor(kValueParamName, u32(i))));
                    }
                    return body;
                });
            },
            [&](const core::type::Struct* str) {
                // Call a helper function that assigns each member separately.
                return call_helper([&] {
                    tint::Vector<const Statement*, 8> body;
                    for (auto member : str->Members()) {
                        auto name = member->Name().Name();
                        body.Push(MakeAssignment(member->Type(),
                                                 b.MemberAccessor(b.Deref(kDestParamName), name),
                                                 b.MemberAccessor(kValueParamName, name)));
                    }
                    return body;
                });
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Checks if a type contains padding bytes.
    /// @param ty the type to check
    /// @returns true if `ty` (or any of its contained types) have padding bytes
    bool HasPadding(const core::type::Type* ty) {
        return Switch(
            ty,  //
            [&](const core::type::Array* arr) {
                auto* elem_ty = arr->ElemType();
                if (elem_ty->Size() % elem_ty->Align() > 0) {
                    return true;
                }
                return HasPadding(elem_ty);
            },
            [&](const core::type::Matrix* mat) {
                auto* col_ty = mat->ColumnType();
                if (mat->ColumnStride() > col_ty->Size()) {
                    return true;
                }
                return HasPadding(col_ty);
            },
            [&](const core::type::Struct* str) {
                uint32_t current_offset = 0;
                for (auto* member : str->Members()) {
                    if (member->Offset() > current_offset) {
                        return true;
                    }
                    if (HasPadding(member->Type())) {
                        return true;
                    }
                    current_offset += member->Type()->Size();
                }
                return (current_offset < str->Size());
            },
            [&](Default) { return false; });
    }

  private:
    /// The program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx;
    /// Alias to the semantic info in ctx.src
    const sem::Info& sem = ctx.src->Sem();
    /// Alias to the symbols in ctx.src
    const SymbolTable& sym = ctx.src->Symbols();
    /// Map of semantic types to their assignment helper functions.
    Hashmap<const core::type::Type*, Symbol, 8> helpers;
};

Transform::ApplyResult PreservePadding::Apply(const Program& program,
                                              const DataMap&,
                                              DataMap&) const {
    return State(program).Run();
}

}  // namespace tint::ast::transform
