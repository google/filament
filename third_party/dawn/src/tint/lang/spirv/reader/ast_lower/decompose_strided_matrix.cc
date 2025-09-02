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

#include "src/tint/lang/spirv/reader/ast_lower/decompose_strided_matrix.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/spirv/reader/ast_lower/simplify_pointers.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::DecomposeStridedMatrix);

namespace tint::spirv::reader {
namespace {

/// MatrixInfo describes a matrix member with a custom stride
struct MatrixInfo {
    /// The stride in bytes between columns of the matrix
    uint32_t stride = 0;
    /// The type of the matrix
    const core::type::Matrix* matrix = nullptr;

    /// @returns the identifier of an array that holds an vector column for each row of the matrix.
    ast::Type array(ast::Builder* b) const {
        ast::Type col_type;
        if (matrix->Type()->Is<core::type::F32>()) {
            col_type = b->ty.vec<f32>(matrix->Rows());
        } else if (matrix->Type()->Is<core::type::F16>()) {
            col_type = b->ty.vec<f16>(matrix->Rows());
        } else {
            TINT_UNREACHABLE();
        }
        return b->ty.array(col_type, u32(matrix->Columns()),
                           Vector{
                               b->Stride(stride),
                           });
    }

    /// Equality operator
    bool operator==(const MatrixInfo& info) const {
        return stride == info.stride && matrix == info.matrix;
    }
    /// Hash function
    struct Hasher {
        size_t operator()(const MatrixInfo& t) const { return Hash(t.stride, t.matrix); }
    };
};

}  // namespace

DecomposeStridedMatrix::DecomposeStridedMatrix() = default;

DecomposeStridedMatrix::~DecomposeStridedMatrix() = default;

ast::transform::Transform::ApplyResult DecomposeStridedMatrix::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    // Scan the program for all storage and uniform structure matrix members with
    // a custom stride attribute. Replace these matrices with an equivalent array,
    // and populate the `decomposed` map with the members that have been replaced.
    bool made_changes = false;
    Hashmap<const core::type::StructMember*, MatrixInfo, 8> decomposed;
    for (auto* node : src.ASTNodes().Objects()) {
        if (auto* str = node->As<ast::Struct>()) {
            auto* str_ty = src.Sem().Get(str);
            if (!str_ty->UsedAs(core::AddressSpace::kUniform) &&
                !str_ty->UsedAs(core::AddressSpace::kStorage)) {
                continue;
            }
            for (auto* member : str_ty->Members()) {
                auto* attr =
                    ast::GetAttribute<ast::StrideAttribute>(member->Declaration()->attributes);
                if (!attr) {
                    // No stride attribute - nothing to do.
                    continue;
                }

                // Get the matrix type, which may be nested inside an array.
                auto* ty = member->Type();
                while (auto* arr = ty->As<core::type::Array>()) {
                    ty = arr->ElemType();
                }
                auto* matrix = ty->As<core::type::Matrix>();
                TINT_ASSERT(matrix);

                made_changes = true;

                uint32_t stride = attr->stride;
                if (matrix->ColumnStride() == stride) {
                    // The attribute specifies the natural stride, so just remove the attribute.
                    auto* disable_validation = ast::GetAttribute<ast::DisableValidationAttribute>(
                        member->Declaration()->attributes);
                    TINT_ASSERT(disable_validation->validation ==
                                ast::DisabledValidation::kIgnoreStrideAttribute);
                    ctx.Remove(member->Declaration()->attributes, attr);
                    ctx.Remove(member->Declaration()->attributes, disable_validation);
                    continue;
                }

                if (member->Type()->Is<core::type::Array>()) {
                    b.Diagnostics().AddError(attr->source)
                        << "custom matrix strides not currently supported on array of matrices";
                    return Program(std::move(b));
                }

                // We've got ourselves a struct member of a matrix type with a custom
                // stride. Replace this with an array of column vectors.
                MatrixInfo info{stride, matrix};
                auto* replacement =
                    b.Member(member->Offset(), ctx.Clone(member->Name()), info.array(ctx.dst));
                ctx.Replace(member->Declaration(), replacement);
                decomposed.Add(member, info);
            }
        }
    }

    if (!made_changes) {
        return SkipTransform;
    }

    // For all expressions where a single matrix column vector was indexed, we can
    // preserve these without calling conversion functions.
    // Example:
    //   ssbo.mat[2] -> ssbo.mat[2]
    ctx.ReplaceAll(
        [&](const ast::IndexAccessorExpression* expr) -> const ast::IndexAccessorExpression* {
            if (auto* access = src.Sem().Get<sem::StructMemberAccess>(expr->object)) {
                if (decomposed.Contains(access->Member())) {
                    auto* obj = ctx.CloneWithoutTransform(expr->object);
                    auto* idx = ctx.Clone(expr->index);
                    return b.IndexAccessor(obj, idx);
                }
            }
            return nullptr;
        });

    // For all struct member accesses to the matrix on the LHS of an assignment,
    // we need to convert the matrix to the array before assigning to the
    // structure.
    // Example:
    //   ssbo.mat = mat_to_arr(m)
    std::unordered_map<MatrixInfo, Symbol, MatrixInfo::Hasher> mat_to_arr;
    ctx.ReplaceAll([&](const ast::AssignmentStatement* stmt) -> const ast::Statement* {
        if (auto* access = src.Sem().Get<sem::StructMemberAccess>(stmt->lhs)) {
            if (auto info = decomposed.Get(access->Member())) {
                auto fn = tint::GetOrAdd(mat_to_arr, *info, [&] {
                    auto name =
                        b.Symbols().New("mat" + std::to_string(info->matrix->Columns()) + "x" +
                                        std::to_string(info->matrix->Rows()) + "_stride_" +
                                        std::to_string(info->stride) + "_to_arr");

                    auto matrix = [&] { return CreateASTTypeFor(ctx, info->matrix); };
                    auto array = [&] { return info->array(ctx.dst); };

                    auto mat = b.Sym("m");
                    Vector<const ast::Expression*, 4> columns;
                    for (uint32_t i = 0; i < static_cast<uint32_t>(info->matrix->Columns()); i++) {
                        columns.Push(b.IndexAccessor(mat, u32(i)));
                    }
                    b.Func(name,
                           Vector{
                               b.Param(mat, matrix()),
                           },
                           array(),
                           Vector{
                               b.Return(b.Call(array(), columns)),
                           });
                    return name;
                });
                auto* lhs = ctx.CloneWithoutTransform(stmt->lhs);
                auto* rhs = b.Call(fn, ctx.Clone(stmt->rhs));
                return b.Assign(lhs, rhs);
            }
        }
        return nullptr;
    });

    // For all other struct member accesses, we need to convert the array to the
    // matrix type. Example:
    //   m = arr_to_mat(ssbo.mat)
    std::unordered_map<MatrixInfo, Symbol, MatrixInfo::Hasher> arr_to_mat;
    ctx.ReplaceAll([&](const ast::MemberAccessorExpression* expr) -> const ast::Expression* {
        if (auto* access = src.Sem().Get(expr)->UnwrapLoad()->As<sem::StructMemberAccess>()) {
            if (auto info = decomposed.Get(access->Member())) {
                auto fn = tint::GetOrAdd(arr_to_mat, *info, [&] {
                    auto name =
                        b.Symbols().New("arr_to_mat" + std::to_string(info->matrix->Columns()) +
                                        "x" + std::to_string(info->matrix->Rows()) + "_stride_" +
                                        std::to_string(info->stride));

                    auto matrix = [&] { return CreateASTTypeFor(ctx, info->matrix); };
                    auto array = [&] { return info->array(ctx.dst); };

                    auto arr = b.Sym("arr");
                    Vector<const ast::Expression*, 4> columns;
                    for (uint32_t i = 0; i < static_cast<uint32_t>(info->matrix->Columns()); i++) {
                        columns.Push(b.IndexAccessor(arr, u32(i)));
                    }
                    b.Func(name,
                           Vector{
                               b.Param(arr, array()),
                           },
                           matrix(),
                           Vector{
                               b.Return(b.Call(matrix(), columns)),
                           });
                    return name;
                });
                return b.Call(fn, ctx.CloneWithoutTransform(expr));
            }
        }
        return nullptr;
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::spirv::reader
