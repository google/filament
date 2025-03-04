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

#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/text/string.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::BindingRemapper);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::BindingRemapper::Remappings);

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::ast::transform {

BindingRemapper::Remappings::Remappings() = default;
BindingRemapper::Remappings::Remappings(BindingPoints bp, AccessControls ac, bool may_collide)
    : binding_points(std::move(bp)),
      access_controls(std::move(ac)),
      allow_collisions(may_collide) {}

BindingRemapper::Remappings::Remappings(const Remappings&) = default;
BindingRemapper::Remappings::~Remappings() = default;

BindingRemapper::BindingRemapper() = default;
BindingRemapper::~BindingRemapper() = default;

Transform::ApplyResult BindingRemapper::Apply(const Program& src,
                                              const DataMap& inputs,
                                              DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    auto* remappings = inputs.Get<Remappings>();
    if (!remappings) {
        b.Diagnostics().AddError(Source{}) << "missing transform data for " << TypeInfo().name;
        return resolver::Resolve(b);
    }

    if (!remappings->allow_collisions && remappings->binding_points.empty() &&
        remappings->access_controls.empty()) {
        return SkipTransform;
    }

    for (auto* var : src.AST().Globals<Var>()) {
        if (var->HasBindingPoint()) {
            auto* global_sem = src.Sem().Get<sem::GlobalVariable>(var);

            // The original binding point
            BindingPoint from = *global_sem->Attributes().binding_point;

            // The binding point after remapping
            BindingPoint bp = from;

            // Replace any group or binding attributes.
            // Note: This has to be performed *before* remapping access controls, as
            // `ctx.Clone(var->attributes)` depend on these replacements.
            auto bp_it = remappings->binding_points.find(from);
            if (bp_it != remappings->binding_points.end()) {
                BindingPoint to = bp_it->second;
                auto* new_group = b.Group(AInt(to.group));
                auto* new_binding = b.Binding(AInt(to.binding));

                auto* old_group = GetAttribute<GroupAttribute>(var->attributes);
                auto* old_binding = GetAttribute<BindingAttribute>(var->attributes);

                ctx.Replace(old_group, new_group);
                ctx.Replace(old_binding, new_binding);
                bp = to;
            }
            // Add `DisableValidationAttribute`s if required
            if (remappings->allow_collisions) {
                auto* attribute = b.Disable(DisabledValidation::kBindingPointCollision);
                ctx.InsertBefore(var->attributes, *var->attributes.begin(), attribute);
            }

            // Replace any access controls.
            auto ac_it = remappings->access_controls.find(from);
            if (ac_it != remappings->access_controls.end()) {
                core::Access access = ac_it->second;
                if (access == core::Access::kUndefined) {
                    b.Diagnostics().AddError(Source{})
                        << "invalid access mode (" << static_cast<uint32_t>(access) << ")";
                    return resolver::Resolve(b);
                }
                auto* sem = src.Sem().Get(var);
                if (sem->AddressSpace() != core::AddressSpace::kStorage) {
                    b.Diagnostics().AddError(Source{})
                        << "cannot apply access control to variable with address space "
                        << sem->AddressSpace();
                    return resolver::Resolve(b);
                }
                auto* ty = sem->Type()->UnwrapRef();
                auto inner_ty = CreateASTTypeFor(ctx, ty);
                auto* new_var =
                    b.create<Var>(ctx.Clone(var->source),                  // source
                                  b.Ident(ctx.Clone(var->name->symbol)),   // name
                                  inner_ty,                                // type
                                  ctx.Clone(var->declared_address_space),  // address space
                                  b.Expr(access),                          // access
                                  ctx.Clone(var->initializer),             // initializer
                                  ctx.Clone(var->attributes));             // attributes
                ctx.Replace(var, new_var);
            }
        }
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
