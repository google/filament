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

#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"

#include <functional>
#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SubstituteOverride);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SubstituteOverride::Config);

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* node : program.AST().GlobalVariables()) {
        if (node->Is<Override>()) {
            return true;
        }
    }
    return false;
}

}  // namespace

SubstituteOverride::SubstituteOverride() = default;

SubstituteOverride::~SubstituteOverride() = default;

Transform::ApplyResult SubstituteOverride::Apply(const Program& src,
                                                 const DataMap& config,
                                                 DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    const auto* data = config.Get<Config>();
    if (!data) {
        b.Diagnostics().AddError(Source{}) << "Missing override substitution data";
        return resolver::Resolve(b);
    }

    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ctx.ReplaceAll([&](const Override* w) -> const Const* {
        auto* sem = ctx.src->Sem().Get(w);

        auto source = ctx.Clone(w->source);
        auto sym = ctx.Clone(w->name->symbol);
        Type ty = w->type ? ctx.Clone(w->type) : Type{};

        // No replacement provided, just clone the override node as a const.
        auto iter = data->map.find(sem->Attributes().override_id.value());
        if (iter == data->map.end()) {
            if (!w->initializer) {
                b.Diagnostics().AddError(Source{})
                    << "Initializer not provided for override, and override not overridden.";
                return nullptr;
            }
            return b.Const(source, sym, ty, ctx.Clone(w->initializer));
        }

        auto value = iter->second;
        auto* ctor = Switch(
            sem->Type(),
            [&](const core::type::Bool*) { return b.Expr(!std::equal_to<double>()(value, 0.0)); },
            [&](const core::type::I32*) { return b.Expr(i32(value)); },
            [&](const core::type::U32*) { return b.Expr(u32(value)); },
            [&](const core::type::F32*) { return b.Expr(f32(value)); },
            [&](const core::type::F16*) { return b.Expr(f16(value)); });

        if (!ctor) {
            b.Diagnostics().AddError(Source{}) << "Failed to create override-expression";
            return nullptr;
        }

        return b.Const(source, sym, ty, ctor);
    });

    // Ensure that objects that are indexed with an override-expression are materialized.
    // If the object is not materialized, and the 'override' variable is turned to a 'const', the
    // resulting type of the index may change. See: crbug.com/tint/1697.
    ctx.ReplaceAll([&](const IndexAccessorExpression* expr) -> const IndexAccessorExpression* {
        if (auto* sem = src.Sem().Get(expr)) {
            if (auto* access = sem->UnwrapMaterialize()->As<sem::IndexAccessorExpression>()) {
                if (access->Object()->UnwrapMaterialize()->Type()->IsAbstract() &&
                    access->Index()->Stage() == core::EvaluationStage::kOverride) {
                    auto* obj = b.Call(wgsl::str(wgsl::BuiltinFn::kTintMaterialize),
                                       ctx.Clone(expr->object));
                    return b.IndexAccessor(obj, ctx.Clone(expr->index));
                }
            }
        }
        return nullptr;
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

SubstituteOverride::Config::Config() = default;

SubstituteOverride::Config::Config(const Config&) = default;

SubstituteOverride::Config::~Config() = default;

SubstituteOverride::Config& SubstituteOverride::Config::operator=(const Config&) = default;

}  // namespace tint::ast::transform
