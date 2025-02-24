// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_lower/transpose_row_major.h"

#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::TransposeRowMajor);

namespace tint::spirv::reader {

TransposeRowMajor::TransposeRowMajor() = default;

TransposeRowMajor::~TransposeRowMajor() = default;

/// PIMPL state for the transform.
struct TransposeRowMajor::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// The semantic info.
    const sem::Info& sem = src.Sem();

    /// Map from array type to a helper function that transposes from a row-major array of matrices.
    Hashmap<const core::type::Array*, Symbol, 4> array_from_row_major_helpers;

    /// Map from array type to a helper function that transposes to a row-major array of matrices.
    Hashmap<const core::type::Array*, Symbol, 4> array_to_row_major_helpers;

    /// Map from matrix reference to column load helper function.
    Hashmap<const core::type::Type*, Symbol, 4> column_load_helpers;

    /// Map from matrix reference to column store helper function.
    Hashmap<const core::type::Type*, Symbol, 4> column_store_helpers;

    /// MatrixLayout describes whether a matrix is row-major or column-major.
    enum class MatrixLayout : uint8_t {
        kRowMajor,
        kColumnMajor,
    };

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    ApplyResult Run() {
        // Scan the program for all storage and uniform structure matrix members with the @row_major
        // attribute. Replace these matrices with a transposed version, and populate the
        // `transposed_members` map with the members that have been replaced.
        Hashset<const core::type::StructMember*, 8> transposed_members;
        for (auto* decl : src.AST().TypeDecls()) {
            if (auto* str = decl->As<ast::Struct>()) {
                auto* str_ty = src.Sem().Get(str);
                if (!str_ty->UsedAs(core::AddressSpace::kUniform) &&
                    !str_ty->UsedAs(core::AddressSpace::kStorage)) {
                    continue;
                }
                for (auto* member : str_ty->Members()) {
                    auto* attr = ast::GetAttribute<ast::RowMajorAttribute>(
                        member->Declaration()->attributes);
                    if (!attr) {
                        continue;
                    }
                    // We've got a struct member of a matrix or array of matrix type with a
                    // row-major memory layout.
                    // Transpose it, remove the @row_major attribute, and record it in the set.
                    auto transposed_matrix = TransposeType(member->Type());
                    ctx.Remove(member->Declaration()->attributes, attr);
                    auto* replacement = b.Member(ctx.Clone(member->Name()), transposed_matrix,
                                                 ctx.Clone(member->Declaration()->attributes));
                    ctx.Replace(member->Declaration(), replacement);
                    transposed_members.Add(member);
                }
            }
        }

        if (transposed_members.IsEmpty()) {
            return SkipTransform;
        }

        // Look for expressions that access the matrix.
        // The `row_major_accesses` set tracks expressions that are accessing a transposed matrix.
        Hashset<const sem::ValueExpression*, 8> row_major_accesses;
        for (auto* node : src.ASTNodes().Objects()) {
            // Check for assignments to all or part of a transposed matrix and replace them.
            if (auto* assign = node->As<ast::AssignmentStatement>()) {
                auto* lhs = src.Sem().GetVal(assign->lhs);
                if (row_major_accesses.Contains(lhs)) {
                    ReplaceAssignment(assign);
                    row_major_accesses.Remove(lhs);
                }
            }

            auto* sem_expr = sem.GetVal(node);
            if (!sem_expr) {
                continue;
            }

            if (auto* accessor = sem_expr->UnwrapLoad()->As<sem::AccessorExpression>()) {
                if (auto* member_access = accessor->As<sem::StructMemberAccess>()) {
                    // Check if we are accessing a struct member that is a transposed matrix.
                    if (transposed_members.Contains(member_access->Member())) {
                        if (member_access->Type()->Is<core::type::MemoryView>()) {
                            // This is a pointer, so track the access until we hit a load or store.
                            row_major_accesses.Add(member_access);
                        } else {
                            // This is not a pointer, so we are extracting a matrix from a value
                            // type. Transpose the matrix now so that all child expressions behave
                            // as expected.
                            ctx.Replace(
                                member_access->Declaration(),
                                b.Call("transpose", ctx.Clone(member_access->Declaration())));
                        }
                    }
                } else {
                    // For non-struct-member accesses, check if the base object is a transposed
                    // matrix and track the resulting sub-expression if so.
                    if (row_major_accesses.Contains(accessor->Object())) {
                        row_major_accesses.Add(accessor);
                        row_major_accesses.Remove(accessor->Object());
                    }
                }
            }

            // Check for loads from all or part of a transposed matrix and replace them.
            if (auto* load = sem_expr->As<sem::Load>()) {
                if (row_major_accesses.Contains(load->Source())) {
                    ReplaceLoad(load);
                    row_major_accesses.Remove(load->Source());
                }
            }

            // Check for constructors of structures that contain transposed matrices, and transpose
            // the relevant arguments.
            if (auto* call = sem_expr->As<sem::Call>()) {
                if (auto* str = call->Type()->As<core::type::Struct>()) {
                    for (uint32_t i = 0; i < str->Members().Length(); i++) {
                        if (transposed_members.Contains(str->Members()[i])) {
                            auto* arg = call->Arguments()[i]->Declaration();
                            ctx.Replace(arg, b.Call("transpose", ctx.Clone(arg)));
                        }
                    }
                }
            }
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Transpose a matrix or array of matrix type.
    /// @param ty the original type
    /// @returns the transposed type
    ast::Type TransposeType(const core::type::Type* ty) {
        if (auto* matrix = ty->As<core::type::Matrix>()) {
            return b.ty.mat(CreateASTTypeFor(ctx, matrix->Type()), matrix->Rows(),
                            matrix->Columns());
        }
        if (auto* arr = ty->As<core::type::Array>()) {
            auto* stride = b.Stride(arr->Stride());
            if (auto count = arr->ConstantCount()) {
                return b.ty.array(TransposeType(arr->ElemType()), b.Expr(u32(count.value())),
                                  Vector{stride});
            }
            return b.ty.array(TransposeType(arr->ElemType()), Vector{stride});
        }
        TINT_UNREACHABLE();
    }

    /// Replace an assignment to a transposed matrix.
    /// @param assign the assignment statement to replace
    void ReplaceAssignment(const ast::AssignmentStatement* assign) {
        auto* lhs = src.Sem().GetVal(assign->lhs);
        Switch(
            src.Sem().GetVal(assign->rhs)->Type(),
            [&](const core::type::Array* arr) {
                // We are storing a whole array, so we need to individually transpose each matrix
                // element. Call a helper function to do this.
                ctx.Replace(assign->rhs,
                            b.Call(TransposeArrayHelper(arr, MatrixLayout::kColumnMajor),
                                   ctx.Clone(assign->rhs)));
            },
            [&](const core::type::Matrix*) {
                // We are storing the whole matrix, so just transpose the RHS.
                ctx.Replace(assign->rhs, b.Call("transpose", ctx.Clone(assign->rhs)));
            },
            [&](const core::type::Vector*) {
                // We are storing a single column, which has to be done element-wise.
                // Call a helper function to do this.
                auto* col_access = lhs->As<sem::IndexAccessorExpression>();
                TINT_ASSERT(col_access);
                auto* to = b.AddressOf(ctx.Clone(col_access->Object()->Declaration()));
                auto* idx = b.Call("u32", ctx.Clone(col_access->Index()->Declaration()));
                auto* col = ctx.Clone(assign->rhs);
                ctx.Replace(assign,
                            b.CallStmt(b.Call(StoreColumnHelper(col_access->Object()->Type()), to,
                                              idx, col)));
            },
            [&](const core::type::Scalar*) {
                // We are storing a single element, so reconstruct the index accessors with the
                // column and row indices swapped over.
                ctx.Replace(assign->lhs, TransposeAccessIndices(lhs));
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get (or create) a helper function that will assign a column to a transposed matrix.
    /// @param dest_type the matrix type we are storing to
    /// @returns the name of the helper function
    Symbol StoreColumnHelper(const core::type::Type* dest_type) {
        auto* ref_type = dest_type->As<core::type::Reference>();
        TINT_ASSERT(ref_type);
        auto* matrix_type = ref_type->StoreType()->As<core::type::Matrix>();
        TINT_ASSERT(matrix_type);
        return column_store_helpers.GetOrAdd(ref_type, [&] {
            // The helper function will look like this:
            //   fn tint_store_row_major(to: ptr<private, mat3x2<f32>>, idx : u32, col: vec3<f32>) {
            //     to[0][idx] = col[0];
            //     to[1][idx] = col[1];
            //     to[2][idx] = col[2];
            //   }
            auto name = b.Symbols().New("tint_store_row_major_column");
            auto transposed = b.ty.mat(CreateASTTypeFor(ctx, matrix_type->Type()),
                                       matrix_type->Rows(), matrix_type->Columns());
            auto ptr = b.ty.ptr(ref_type->AddressSpace(), transposed,
                                ref_type->AddressSpace() == core::AddressSpace::kStorage
                                    ? ref_type->Access()
                                    : core::Access::kUndefined);
            auto* to = b.Param("tint_to", ptr);
            auto* idx = b.Param("tint_idx", b.ty.u32());
            auto* col = b.Param("tint_col", CreateASTTypeFor(ctx, matrix_type->ColumnType()));
            Vector<const ast::Statement*, 4> body;
            for (uint32_t i = 0; i < matrix_type->Rows(); i++) {
                body.Push(b.Assign(b.IndexAccessor(b.IndexAccessor(to, b.Expr(AInt(i))), idx),
                                   b.IndexAccessor(col, b.Expr(AInt(i)))));
            }
            b.Func(name, Vector{to, idx, col}, {}, std::move(body));
            return name;
        });
    }

    /// Replace a load from a transposed matrix.
    /// @param load the load expression to replace
    void ReplaceLoad(const sem::Load* load) {
        Switch(
            load->Type(),
            [&](const core::type::Array* arr) {
                // We are loading a whole array, so we need to individually transpose each matrix
                // element. Call a helper function to do this.
                ctx.Replace(load->Declaration(),
                            b.Call(TransposeArrayHelper(arr, MatrixLayout::kRowMajor),
                                   ctx.Clone(load->Declaration())));
            },
            [&](const core::type::Matrix*) {
                // We are loading the whole matrix, so just transpose the result.
                ctx.Replace(load->Declaration(),
                            b.Call("transpose", ctx.Clone(load->Declaration())));
            },
            [&](const core::type::Vector*) {
                // We are loading a single column, which has to be done element-wise.
                // Call a helper function to do this.
                auto* col_access = load->Source()->As<sem::IndexAccessorExpression>();
                TINT_ASSERT(col_access);
                auto* from = b.AddressOf(ctx.Clone(col_access->Object()->Declaration()));
                auto* idx = b.Call("u32", ctx.Clone(col_access->Index()->Declaration()));
                ctx.Replace(load->Declaration(),
                            b.Call(LoadColumnHelper(col_access->Object()->Type()), from, idx));
            },
            [&](const core::type::Scalar*) {
                // We are loading a single element, so reconstruct the index accessors with the
                // column and row indices swapped over.
                ctx.Replace(load->Declaration(), TransposeAccessIndices(load->Source()));
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get (or create) a helper function that will transpose an array of matrices.
    /// @param arr_type the array type that we are producing
    /// @param src_layout specifies if the source is row-major or column-major
    /// @returns the name of the helper function
    Symbol TransposeArrayHelper(const core::type::Array* arr_type, MatrixLayout src_layout) {
        auto& helpers = src_layout == MatrixLayout::kRowMajor ? array_from_row_major_helpers
                                                              : array_to_row_major_helpers;
        return helpers.GetOrAdd(arr_type, [&] {
            // The helper function will look like this:
            //   fn tint_transpose_array(from: array<mat3x2<f32>, 4>) -> array<mat2x3<f32>, 4> {
            //     var result : array<mat2x3<f32>, 4>;
            //     for (var i = 0; i < 4; i++) {
            //       result[i] = transpose(from[i]);
            //     }
            //     return result;
            //   }
            ast::Type from_param_type;
            ast::Type return_type;
            ast::Type result_type;
            if (src_layout == MatrixLayout::kRowMajor) {
                from_param_type = TransposeType(arr_type);
                return_type = CreateASTTypeFor(ctx, arr_type);
                result_type = CreateASTTypeFor(ctx, arr_type);
            } else {
                from_param_type = CreateASTTypeFor(ctx, arr_type);
                return_type = TransposeType(arr_type);
                result_type = TransposeType(arr_type);
            }
            auto name = b.Symbols().New("tint_transpose_array");
            auto* from = b.Param("tint_from", from_param_type);
            auto result = b.Symbols().New("tint_result");
            auto i = b.Symbols().New("i");
            auto count = arr_type->ConstantCount();

            const ast::Expression* transpose = nullptr;
            if (auto* nested_arr = arr_type->ElemType()->As<core::type::Array>()) {
                transpose =
                    b.Call(TransposeArrayHelper(nested_arr, src_layout), b.IndexAccessor(from, i));
            } else {
                TINT_ASSERT(arr_type->ElemType()->Is<core::type::Matrix>());
                transpose = b.Call("transpose", b.IndexAccessor(from, i));
            }

            b.Func(
                name, Vector{from}, return_type,
                Vector{
                    b.Decl(b.Var(result, result_type)),
                    b.For(b.Decl(b.Var(i, b.Expr(0_u))), b.LessThan(i, u32(*count)), b.Increment(i),
                          b.Block(b.Assign(b.IndexAccessor(result, i), transpose))),
                    b.Return(result),
                });
            return name;
        });
    }

    /// Get (or create) a helper function that will load a column from a transposed matrix.
    /// @param src_type the matrix type we are loading from
    /// @returns the name of the helper function
    Symbol LoadColumnHelper(const core::type::Type* src_type) {
        auto* ref_type = src_type->As<core::type::Reference>();
        TINT_ASSERT(ref_type);
        auto* matrix_type = ref_type->StoreType()->As<core::type::Matrix>();
        TINT_ASSERT(matrix_type);
        return column_load_helpers.GetOrAdd(ref_type, [&] {
            // The helper function will look like this:
            //   fn tint_load_row_major(from: ptr<private, mat3x2<f32>>, idx : u32) -> vec3<f32> {
            //     return vec3<f32>(to[0][idx], to[1][idx], to[2][idx]);
            //   }
            auto name = b.Symbols().New("tint_load_row_major_column");
            auto transposed = b.ty.mat(CreateASTTypeFor(ctx, matrix_type->Type()),
                                       matrix_type->Rows(), matrix_type->Columns());
            auto ptr = b.ty.ptr(ref_type->AddressSpace(), transposed,
                                ref_type->AddressSpace() == core::AddressSpace::kStorage
                                    ? ref_type->Access()
                                    : core::Access::kUndefined);
            auto* from = b.Param("tint_from", ptr);
            auto* idx = b.Param("tint_idx", b.ty.u32());
            Vector<const ast::Expression*, 4> rows;
            for (uint32_t i = 0; i < matrix_type->Rows(); i++) {
                rows.Push(b.IndexAccessor(b.IndexAccessor(from, b.Expr(AInt(i))), idx));
            }
            b.Func(name, Vector{from, idx}, CreateASTTypeFor(ctx, matrix_type->ColumnType()),
                   Vector{
                       b.Return(b.Call(CreateASTTypeFor(ctx, matrix_type->ColumnType()),
                                       std::move(rows))),
                   });
            return name;
        });
    }

    /// Swap the column and row indices for a matrix element accessor chain.
    /// @param expr the accessor expression to transpose
    /// @returns the transposed access chain
    const ast::Expression* TransposeAccessIndices(const sem::ValueExpression* expr) {
        auto* row_access = expr->As<sem::AccessorExpression>();
        TINT_ASSERT(row_access);
        auto* col_access = row_access->Object()->As<sem::IndexAccessorExpression>();
        TINT_ASSERT(col_access);
        auto* matrix = ctx.Clone(col_access->Object()->Declaration());
        auto* col_idx = ctx.Clone(col_access->Index()->Declaration());

        // The row index could either be a array accessor or a vector component swizzle.
        const ast::Expression* row_idx = nullptr;
        if (auto* index = row_access->As<sem::IndexAccessorExpression>()) {
            row_idx = ctx.Clone(index->Index()->Declaration());
        } else if (auto* swizzle = row_access->As<sem::Swizzle>()) {
            row_idx = b.Expr(u32(swizzle->Indices()[0]));
        } else {
            TINT_UNREACHABLE();
        }

        return b.IndexAccessor(b.IndexAccessor(matrix, row_idx), col_idx);
    }
};

ast::transform::Transform::ApplyResult TransposeRowMajor::Apply(const Program& src,
                                                                const ast::transform::DataMap&,
                                                                ast::transform::DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::spirv::reader
