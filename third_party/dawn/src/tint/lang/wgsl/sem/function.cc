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

#include "src/tint/lang/wgsl/sem/function.h"

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Function);

namespace tint::sem {

Function::Function(const ast::Function* declaration)
    : Base(core::EvaluationStage::kRuntime,
           ast::HasAttribute<ast::MustUseAttribute>(declaration->attributes)),
      declaration_(declaration),
      workgroup_size_{1, 1, 1} {}

Function::~Function() = default;

std::vector<std::pair<const Variable*, const ast::LocationAttribute*>>
Function::TransitivelyReferencedLocationVariables() const {
    std::vector<std::pair<const Variable*, const ast::LocationAttribute*>> ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        for (auto* attr : global->Declaration()->attributes) {
            if (auto* location = attr->As<ast::LocationAttribute>()) {
                ret.push_back({global, location});
                break;
            }
        }
    }
    return ret;
}

void Function::AddTransitivelyReferencedGlobal(const sem::GlobalVariable* global) {
    if (transitively_referenced_globals_.Add(global)) {
        for (auto* ref : global->TransitivelyReferencedOverrides()) {
            AddTransitivelyReferencedGlobal(ref);
        }
    }
}

Function::VariableBindings Function::TransitivelyReferencedUniformVariables() const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        if (global->AddressSpace() != core::AddressSpace::kUniform) {
            continue;
        }

        if (auto bp = global->Attributes().binding_point) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedStorageBufferVariables() const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        if (global->AddressSpace() != core::AddressSpace::kStorage) {
            continue;
        }

        if (auto bp = global->Attributes().binding_point) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

std::vector<std::pair<const Variable*, const ast::BuiltinAttribute*>>
Function::TransitivelyReferencedBuiltinVariables() const {
    std::vector<std::pair<const Variable*, const ast::BuiltinAttribute*>> ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        for (auto* attr : global->Declaration()->attributes) {
            if (auto* builtin = attr->As<ast::BuiltinAttribute>()) {
                ret.push_back({global, builtin});
                break;
            }
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariables() const {
    return TransitivelyReferencedSamplerVariablesImpl(core::type::SamplerKind::kSampler);
}

Function::VariableBindings Function::TransitivelyReferencedComparisonSamplerVariables() const {
    return TransitivelyReferencedSamplerVariablesImpl(core::type::SamplerKind::kComparisonSampler);
}

Function::VariableBindings Function::TransitivelyReferencedSampledTextureVariables() const {
    return TransitivelyReferencedSampledTextureVariablesImpl(false);
}

Function::VariableBindings Function::TransitivelyReferencedMultisampledTextureVariables() const {
    return TransitivelyReferencedSampledTextureVariablesImpl(true);
}

Function::VariableBindings Function::TransitivelyReferencedVariablesOfType(
    const tint::TypeInfo* type) const {
    VariableBindings ret;
    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        if (unwrapped_type->TypeInfo().Is(type)) {
            if (auto bp = global->Attributes().binding_point) {
                ret.push_back({global, *bp});
            }
        }
    }
    return ret;
}

bool Function::HasAncestorEntryPoint(Symbol symbol) const {
    for (const auto* point : ancestor_entry_points_) {
        if (point->Declaration()->name->symbol == symbol) {
            return true;
        }
    }
    return false;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariablesImpl(
    core::type::SamplerKind kind) const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        auto* sampler = unwrapped_type->As<core::type::Sampler>();
        if (sampler == nullptr || sampler->Kind() != kind) {
            continue;
        }

        if (auto bp = global->Attributes().binding_point) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedSampledTextureVariablesImpl(
    bool multisampled) const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        auto* texture = unwrapped_type->As<core::type::Texture>();
        if (texture == nullptr) {
            continue;
        }

        auto is_multisampled = texture->Is<core::type::MultisampledTexture>();
        auto is_sampled = texture->Is<core::type::SampledTexture>();

        if ((multisampled && !is_multisampled) || (!multisampled && !is_sampled)) {
            continue;
        }

        if (auto bp = global->Attributes().binding_point) {
            ret.push_back({global, *bp});
        }
    }

    return ret;
}

void Function::SetDiagnosticSeverity(wgsl::DiagnosticRule rule, wgsl::DiagnosticSeverity severity) {
    diagnostic_severities_.Add(rule, severity);
}

}  // namespace tint::sem
