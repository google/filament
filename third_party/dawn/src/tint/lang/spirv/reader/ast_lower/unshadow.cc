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

#include "src/tint/lang/spirv/reader/ast_lower/unshadow.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::Unshadow);

namespace tint::ast::transform {

/// PIMPL state for the transform
struct Unshadow::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    Transform::ApplyResult Run() {
        auto& sem = src.Sem();

        // Maps a variable to its new name.
        Hashmap<const sem::Variable*, Symbol, 8> renamed_to;

        auto rename = [&](const sem::Variable* v) -> const Variable* {
            auto* decl = v->Declaration();
            auto name = decl->name->symbol.Name();
            auto symbol = b.Symbols().New(name);
            renamed_to.Add(v, symbol);

            auto source = ctx.Clone(decl->source);
            auto type = decl->type ? ctx.Clone(decl->type) : Type{};
            auto* initializer = ctx.Clone(decl->initializer);
            auto attributes = ctx.Clone(decl->attributes);
            return Switch(
                decl,  //
                [&](const Var* var) {
                    return b.Var(source, symbol, type, var->declared_address_space,
                                 var->declared_access, initializer, attributes);
                },
                [&](const Let*) { return b.Let(source, symbol, type, initializer, attributes); },
                [&](const Const*) {
                    return b.Const(source, symbol, type, initializer, attributes);
                },
                [&](const Parameter*) {  //
                    return b.Param(source, symbol, type, attributes);
                },  //
                TINT_ICE_ON_NO_MATCH);
        };

        bool made_changes = false;

        for (auto* node : ctx.src->SemNodes().Objects()) {
            Switch(
                node,  //
                [&](const sem::LocalVariable* local) {
                    if (local->Shadows()) {
                        ctx.Replace(local->Declaration(), [&, local] { return rename(local); });
                        made_changes = true;
                    }
                },
                [&](const sem::Parameter* param) {
                    if (param->Shadows()) {
                        ctx.Replace(param->Declaration(), [&, param] { return rename(param); });
                        made_changes = true;
                    }
                });
        }

        if (!made_changes) {
            return SkipTransform;
        }

        ctx.ReplaceAll([&](const IdentifierExpression* ident) -> const IdentifierExpression* {
            if (auto* sem_ident = sem.GetVal(ident)) {
                if (auto* user = sem_ident->Unwrap()->As<sem::VariableUser>()) {
                    if (auto renamed = renamed_to.Get(user->Variable())) {
                        return b.Expr(*renamed);
                    }
                }
            }
            return nullptr;
        });

        ctx.Clone();
        return resolver::Resolve(b);
    }
};

Unshadow::Unshadow() = default;

Unshadow::~Unshadow() = default;

Transform::ApplyResult Unshadow::Apply(const Program& src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::ast::transform
