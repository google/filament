// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/helpers/append_vector.h"

#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl {
namespace {

struct VectorConstructorInfo {
    const sem::Call* call = nullptr;
    const sem::ValueConstructor* ctor = nullptr;
    explicit operator bool() const { return call != nullptr; }
};
VectorConstructorInfo AsVectorConstructor(const sem::ValueExpression* expr) {
    if (auto* call = expr->As<sem::Call>()) {
        if (auto* ctor = call->Target()->As<sem::ValueConstructor>()) {
            if (ctor->ReturnType()->Is<core::type::Vector>()) {
                return {call, ctor};
            }
        }
    }
    return {};
}

const sem::ValueExpression* Zero(ProgramBuilder& b,
                                 const core::type::Type* ty,
                                 const sem::Statement* stmt) {
    const ast::Expression* expr = nullptr;
    if (ty->Is<core::type::I32>()) {
        expr = b.Expr(0_i);
    } else if (ty->Is<core::type::U32>()) {
        expr = b.Expr(0_u);
    } else if (ty->Is<core::type::F32>()) {
        expr = b.Expr(0_f);
    } else if (ty->Is<core::type::Bool>()) {
        expr = b.Expr(false);
    } else {
        TINT_UNREACHABLE() << "unsupported vector element type: " << ty->TypeInfo().name;
    }
    auto* sem = b.create<sem::ValueExpression>(expr, ty, core::EvaluationStage::kRuntime, stmt,
                                               /* constant_value */ nullptr,
                                               /* has_side_effects */ false);
    b.Sem().Add(expr, sem);
    return sem;
}

}  // namespace

const sem::Call* AppendVector(ProgramBuilder* b,
                              const ast::Expression* vector_ast,
                              const ast::Expression* scalar_ast) {
    uint32_t packed_size;
    const core::type::Type* packed_el_sem_ty;
    auto* vector_sem = b->Sem().GetVal(vector_ast);
    auto* scalar_sem = b->Sem().GetVal(scalar_ast);
    auto* vector_ty = vector_sem->Type()->UnwrapRef();
    if (auto* vec = vector_ty->As<core::type::Vector>()) {
        packed_size = vec->Width() + 1;
        packed_el_sem_ty = vec->Type();
    } else {
        packed_size = 2;
        packed_el_sem_ty = vector_ty;
    }

    auto packed_el_ast_ty = Switch(
        packed_el_sem_ty,  //
        [&](const core::type::I32*) { return b->ty.i32(); },
        [&](const core::type::U32*) { return b->ty.u32(); },
        [&](const core::type::F32*) { return b->ty.f32(); },
        [&](const core::type::Bool*) { return b->ty.bool_(); },  //
        TINT_ICE_ON_NO_MATCH);

    auto* statement = vector_sem->Stmt();

    auto packed_ast_ty = b->ty.vec(packed_el_ast_ty, packed_size);
    auto* packed_sem_ty = b->create<core::type::Vector>(packed_el_sem_ty, packed_size);

    // If the coordinates are already passed in a vector constructor, with only
    // scalar components supplied, extract the elements into the new vector
    // instead of nesting a vector-in-vector.
    // If the coordinates are a zero-constructor of the vector, then expand that
    // to scalar zeros.
    // The other cases for a nested vector constructor are when it is used
    // to convert a vector of a different type, e.g. vec2<i32>(vec2<u32>()).
    // In that case, preserve the original argument, or you'll get a type error.

    Vector<const sem::ValueExpression*, 4> packed;
    if (auto vc = AsVectorConstructor(vector_sem)) {
        const auto num_supplied = vc.call->Arguments().Length();
        if (num_supplied == 0) {
            // Zero-value vector constructor. Populate with zeros
            for (uint32_t i = 0; i < packed_size - 1; i++) {
                auto* zero = Zero(*b, packed_el_sem_ty, statement);
                packed.Push(zero);
            }
        } else if (num_supplied + 1 == packed_size) {
            // All vector components were supplied as scalars.  Pass them through.
            packed = vc.call->Arguments();
        }
    }
    if (packed.IsEmpty()) {
        // The special cases didn't occur. Use the vector argument as-is.
        packed.Push(vector_sem);
    }

    if (packed_el_sem_ty != scalar_sem->Type()->UnwrapRef()) {
        // Cast scalar to the vector element type
        auto* scalar_cast_ast = b->Call(packed_el_ast_ty, scalar_ast);
        auto* param = b->create<sem::Parameter>(nullptr, 0u, scalar_sem->Type()->UnwrapRef());
        auto* scalar_cast_target = b->create<sem::ValueConversion>(packed_el_sem_ty, param,
                                                                   core::EvaluationStage::kRuntime);
        auto* scalar_cast_sem = b->create<sem::Call>(
            scalar_cast_ast, scalar_cast_target, core::EvaluationStage::kRuntime,
            Vector<const sem::ValueExpression*, 1>{scalar_sem}, statement,
            /* constant_value */ nullptr, /* has_side_effects */ false);
        b->Sem().Add(scalar_cast_ast, scalar_cast_sem);
        packed.Push(scalar_cast_sem);
    } else {
        packed.Push(scalar_sem);
    }

    auto* ctor_ast =
        b->Call(packed_ast_ty, tint::Transform(packed, [&](const sem::ValueExpression* expr) {
                    return expr->Declaration();
                }));
    auto* ctor_target = b->create<sem::ValueConstructor>(
        packed_sem_ty,
        tint::Transform(packed,
                        [&](const tint::sem::ValueExpression* arg, size_t i) {
                            return b->create<sem::Parameter>(nullptr, static_cast<uint32_t>(i),
                                                             arg->Type()->UnwrapRef());
                        }),
        core::EvaluationStage::kRuntime);
    auto* ctor_sem = b->create<sem::Call>(ctor_ast, ctor_target, core::EvaluationStage::kRuntime,
                                          std::move(packed), statement,
                                          /* constant_value */ nullptr,
                                          /* has_side_effects */ false);
    b->Sem().Add(ctor_ast, ctor_sem);
    return ctor_sem;
}

}  // namespace tint::wgsl
