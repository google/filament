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

#include "src/tint/lang/wgsl/ast/transform/single_entry_point.h"

#include <unordered_set>
#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SingleEntryPoint);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SingleEntryPoint::Config);

namespace tint::ast::transform {

SingleEntryPoint::SingleEntryPoint() = default;

SingleEntryPoint::~SingleEntryPoint() = default;

Transform::ApplyResult SingleEntryPoint::Apply(const Program& src,
                                               const DataMap& inputs,
                                               DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        b.Diagnostics().AddError(Source{}) << "missing transform data for " << TypeInfo().name;
        return resolver::Resolve(b);
    }

    // Find the target entry point.
    const Function* entry_point = nullptr;
    for (auto* f : src.AST().Functions()) {
        if (!f->IsEntryPoint()) {
            continue;
        }
        if (f->name->symbol.Name() == cfg->entry_point_name) {
            entry_point = f;
            break;
        }
    }
    if (entry_point == nullptr) {
        b.Diagnostics().AddError(Source{})
            << "entry point '" << cfg->entry_point_name << "' not found";
        return resolver::Resolve(b);
    }

    auto& sem = src.Sem();
    auto& referenced_vars = sem.Get(entry_point)->TransitivelyReferencedGlobals();

    // Clone any module-scope variables, types, and functions that are statically referenced by the
    // target entry point.
    for (auto* decl : src.AST().GlobalDeclarations()) {
        Switch(
            decl,  //
            [&](const TypeDecl* ty) {
                // Strip aliases that reference unused override declarations.
                if (auto* arr = sem.Get(ty)->As<sem::Array>()) {
                    for (auto* o : arr->TransitivelyReferencedOverrides()) {
                        if (!referenced_vars.Contains(o)) {
                            return;
                        }
                    }
                }

                // TODO(jrprice): Strip other unused types.
                b.AST().AddTypeDecl(ctx.Clone(ty));
            },
            [&](const Override* override) {
                if (referenced_vars.Contains(sem.Get(override))) {
                    if (!HasAttribute<IdAttribute>(override->attributes)) {
                        // If the override doesn't already have an @id() attribute, add one
                        // so that its allocated ID so that it won't be affected by other
                        // stripped away overrides
                        auto* global = sem.Get(override);
                        const auto* id = b.Id(global->Attributes().override_id.value());
                        ctx.InsertFront(override->attributes, id);
                    }
                    b.AST().AddGlobalVariable(ctx.Clone(override));
                }
            },
            [&](const Var* var) {
                if (referenced_vars.Contains(sem.Get<sem::GlobalVariable>(var))) {
                    b.AST().AddGlobalVariable(ctx.Clone(var));
                }
            },
            [&](const Const* c) {
                // Always keep 'const' declarations, as these can be used by attributes and array
                // sizes, which are not tracked as transitively used by functions. They also don't
                // typically get emitted by the backend unless they're actually used.
                b.AST().AddGlobalVariable(ctx.Clone(c));
            },
            [&](const Function* func) {
                if (sem.Get(func)->HasAncestorEntryPoint(entry_point->name->symbol)) {
                    b.AST().AddFunction(ctx.Clone(func));
                }
            },
            [&](const Enable* ext) { b.AST().AddEnable(ctx.Clone(ext)); },
            [&](const Requires*) {
                // Drop requires directives as they are optional, and it's non-trivial to determine
                // which features are needed for which entry points.
            },
            [&](const ConstAssert* ca) { b.AST().AddConstAssert(ctx.Clone(ca)); },
            [&](const DiagnosticDirective* d) { b.AST().AddDiagnosticDirective(ctx.Clone(d)); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    // Clone the entry point.
    b.AST().AddFunction(ctx.Clone(entry_point));

    return resolver::Resolve(b);
}

SingleEntryPoint::Config::Config(std::string entry_point) : entry_point_name(entry_point) {}

SingleEntryPoint::Config::Config(const Config&) = default;
SingleEntryPoint::Config::~Config() = default;
SingleEntryPoint::Config& SingleEntryPoint::Config::operator=(const Config&) = default;

}  // namespace tint::ast::transform
