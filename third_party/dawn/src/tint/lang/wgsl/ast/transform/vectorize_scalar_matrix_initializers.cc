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

#include "src/tint/lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::VectorizeScalarMatrixInitializers);

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* node : program.ASTNodes().Objects()) {
        if (auto* call = program.Sem().Get<sem::Call>(node)) {
            if (call->Target()->Is<sem::ValueConstructor>() &&
                call->Type()->Is<core::type::Matrix>()) {
                auto& args = call->Arguments();
                if (!args.IsEmpty() && args[0]->Type()->UnwrapRef()->Is<core::type::Scalar>()) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

VectorizeScalarMatrixInitializers::VectorizeScalarMatrixInitializers() = default;

VectorizeScalarMatrixInitializers::~VectorizeScalarMatrixInitializers() = default;

Transform::ApplyResult VectorizeScalarMatrixInitializers::Apply(const Program& src,
                                                                const DataMap&,
                                                                DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    std::unordered_map<const core::type::Matrix*, Symbol> scalar_inits;

    ctx.ReplaceAll([&](const CallExpression* expr) -> const CallExpression* {
        auto* call = src.Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_init = call->Target()->As<sem::ValueConstructor>();
        if (!ty_init) {
            return nullptr;
        }
        auto* mat_type = call->Type()->As<core::type::Matrix>();
        if (!mat_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.IsEmpty()) {
            return nullptr;
        }

        // If the argument type is a matrix, then this is an identity / conversion initializer.
        // If the argument type is a vector, then we're already column vectors.
        // If the argument type is abstract, then we're const-expression and there's no need to
        // adjust this, as it'll be constant folded by the backend.
        if (args[0]
                ->Type()
                ->UnwrapRef()
                ->IsAnyOf<core::type::Matrix, core::type::Vector, core::type::AbstractNumeric>()) {
            return nullptr;
        }

        // Constructs a matrix using vector columns, with the elements constructed using the
        // 'element(uint32_t c, uint32_t r)' callback.
        auto build_mat = [&](auto&& element) {
            tint::Vector<const Expression*, 4> columns;
            for (uint32_t c = 0; c < mat_type->Columns(); c++) {
                tint::Vector<const Expression*, 4> row_values;
                for (uint32_t r = 0; r < mat_type->Rows(); r++) {
                    row_values.Push(element(c, r));
                }

                // Construct the column vector.
                columns.Push(b.vec(CreateASTTypeFor(ctx, mat_type->Type()), mat_type->Rows(),
                                   std::move(row_values)));
            }
            return b.Call(CreateASTTypeFor(ctx, mat_type), columns);
        };

        if (args.Length() == 1) {
            // Generate a helper function for constructing the matrix.
            // This is done to ensure that the single argument value is only evaluated once, and
            // with the correct expression evaluation order.
            auto fn = tint::GetOrAdd(scalar_inits, mat_type, [&] {
                auto name = b.Symbols().New("build_mat" + std::to_string(mat_type->Columns()) +
                                            "x" + std::to_string(mat_type->Rows()));
                b.Func(name,
                       tint::Vector{
                           // Single scalar parameter
                           b.Param("value", CreateASTTypeFor(ctx, mat_type->Type())),
                       },
                       CreateASTTypeFor(ctx, mat_type),
                       tint::Vector{
                           b.Return(build_mat([&](uint32_t, uint32_t) {  //
                               return b.Expr("value");
                           })),
                       });
                return name;
            });
            return b.Call(fn, ctx.Clone(args[0]->Declaration()));
        }

        if (DAWN_LIKELY(args.Length() == mat_type->Columns() * mat_type->Rows())) {
            return build_mat([&](uint32_t c, uint32_t r) {
                return ctx.Clone(args[c * mat_type->Rows() + r]->Declaration());
            });
        }

        TINT_ICE() << "matrix initializer has unexpected number of arguments";
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
