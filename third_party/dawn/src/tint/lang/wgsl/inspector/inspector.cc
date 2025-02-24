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

#include "src/tint/lang/wgsl/inspector/inspector.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/interpolation_sampling.h"
#include "src/tint/lang/core/interpolation_type.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::inspector {
namespace {

void AppendResourceBindings(std::vector<ResourceBinding>* dest,
                            const std::vector<ResourceBinding>& orig) {
    TINT_ASSERT(dest);
    if (!dest) {
        return;
    }

    dest->reserve(dest->size() + orig.size());
    dest->insert(dest->end(), orig.begin(), orig.end());
}

std::tuple<ComponentType, CompositionType> CalculateComponentAndComposition(
    const core::type::Type* type) {
    // entry point in/out variables must of numeric scalar or vector types.
    TINT_ASSERT(type->IsNumericScalarOrVector());

    ComponentType componentType = Switch(
        type->DeepestElement(),  //
        [&](const core::type::F32*) { return ComponentType::kF32; },
        [&](const core::type::F16*) { return ComponentType::kF16; },
        [&](const core::type::I32*) { return ComponentType::kI32; },
        [&](const core::type::U32*) { return ComponentType::kU32; },  //
        TINT_ICE_ON_NO_MATCH);

    CompositionType compositionType;
    if (auto* vec = type->As<core::type::Vector>()) {
        switch (vec->Width()) {
            case 2: {
                compositionType = CompositionType::kVec2;
                break;
            }
            case 3: {
                compositionType = CompositionType::kVec3;
                break;
            }
            case 4: {
                compositionType = CompositionType::kVec4;
                break;
            }
            default: {
                TINT_UNREACHABLE() << "unhandled composition type";
            }
        }
    } else {
        compositionType = CompositionType::kScalar;
    }

    return {componentType, compositionType};
}

bool IsTextureBuiltinThatUsesNonComparisonSampler(tint::wgsl::BuiltinFn builtin_fn) {
    return builtin_fn == wgsl::BuiltinFn::kTextureSample ||
           builtin_fn == wgsl::BuiltinFn::kTextureSampleLevel ||
           builtin_fn == wgsl::BuiltinFn::kTextureGather;
}

}  // namespace

Inspector::Inspector(const Program& program) : program_(program) {}

Inspector::~Inspector() = default;

EntryPoint Inspector::GetEntryPoint(const tint::ast::Function* func) {
    EntryPoint entry_point;
    TINT_ASSERT(func != nullptr);
    TINT_ASSERT(func->IsEntryPoint());

    auto* sem = program_.Sem().Get(func);

    entry_point.name = func->name->symbol.Name();
    entry_point.remapped_name = func->name->symbol.Name();

    switch (func->PipelineStage()) {
        case ast::PipelineStage::kCompute: {
            entry_point.stage = PipelineStage::kCompute;
            entry_point.workgroup_storage_size = ComputeWorkgroupStorageSize(func);

            auto wgsize = sem->WorkgroupSize();
            if (wgsize[0].has_value() && wgsize[1].has_value() && wgsize[2].has_value()) {
                entry_point.workgroup_size = {wgsize[0].value(), wgsize[1].value(),
                                              wgsize[2].value()};
            }
            break;
        }
        case ast::PipelineStage::kFragment: {
            entry_point.stage = PipelineStage::kFragment;
            entry_point.pixel_local_members = ComputePixelLocalMemberTypes(func);
            break;
        }
        case ast::PipelineStage::kVertex: {
            entry_point.stage = PipelineStage::kVertex;
            break;
        }
        default: {
            TINT_UNREACHABLE() << "invalid pipeline stage for entry point '" << entry_point.name
                               << "'";
        }
    }

    entry_point.push_constant_size = ComputePushConstantSize(func);

    for (auto* param : sem->Parameters()) {
        AddEntryPointInOutVariables(
            param->Declaration()->name->symbol.Name(), param->Declaration()->name->symbol.Name(),
            param->Type(), param->Declaration()->attributes, param->Attributes().location,
            param->Attributes().color, /* @blend_src */ std::nullopt, entry_point.input_variables);

        entry_point.input_position_used |= ContainsBuiltin(
            core::BuiltinValue::kPosition, param->Type(), param->Declaration()->attributes);
        entry_point.front_facing_used |= ContainsBuiltin(
            core::BuiltinValue::kFrontFacing, param->Type(), param->Declaration()->attributes);
        entry_point.sample_index_used |= ContainsBuiltin(
            core::BuiltinValue::kSampleIndex, param->Type(), param->Declaration()->attributes);
        entry_point.input_sample_mask_used |= ContainsBuiltin(
            core::BuiltinValue::kSampleMask, param->Type(), param->Declaration()->attributes);
        entry_point.num_workgroups_used |= ContainsBuiltin(
            core::BuiltinValue::kNumWorkgroups, param->Type(), param->Declaration()->attributes);
        entry_point.vertex_index_used |= ContainsBuiltin(
            core::BuiltinValue::kVertexIndex, param->Type(), param->Declaration()->attributes);
        entry_point.instance_index_used |= ContainsBuiltin(
            core::BuiltinValue::kInstanceIndex, param->Type(), param->Declaration()->attributes);
    }

    if (!sem->ReturnType()->Is<core::type::Void>()) {
        AddEntryPointInOutVariables("<retval>", "", sem->ReturnType(), func->return_type_attributes,
                                    sem->ReturnLocation(), /* @color */ std::nullopt,
                                    /* @blend_src */ std::nullopt, entry_point.output_variables);

        entry_point.output_sample_mask_used = ContainsBuiltin(
            core::BuiltinValue::kSampleMask, sem->ReturnType(), func->return_type_attributes);
        entry_point.frag_depth_used = ContainsBuiltin(
            core::BuiltinValue::kFragDepth, sem->ReturnType(), func->return_type_attributes);
        entry_point.clip_distances_size = GetClipDistancesBuiltinSize(sem->ReturnType());
    }

    for (auto* var : sem->TransitivelyReferencedGlobals()) {
        auto* decl = var->Declaration();

        auto name = decl->name->symbol.Name();

        auto* global = var->As<sem::GlobalVariable>();
        if (auto override_id = global->Attributes().override_id) {
            Override override;
            override.name = name;
            override.id = override_id.value();
            auto* type = var->Type();
            TINT_ASSERT(type->Is<core::type::Scalar>());
            if (type->IsBoolScalarOrVector()) {
                override.type = Override::Type::kBool;
            } else if (type->IsFloatScalar()) {
                if (type->Is<core::type::F16>()) {
                    override.type = Override::Type::kFloat16;
                } else {
                    override.type = Override::Type::kFloat32;
                }
            } else if (type->IsSignedIntegerScalar()) {
                override.type = Override::Type::kInt32;
            } else if (type->IsUnsignedIntegerScalar()) {
                override.type = Override::Type::kUint32;
            } else {
                TINT_UNREACHABLE();
            }

            override.is_initialized = global->Declaration()->initializer;
            override.is_id_specified =
                ast::HasAttribute<ast::IdAttribute>(global->Declaration()->attributes);

            entry_point.overrides.push_back(override);
        }
    }

    {
        auto filter = [](const tint::sem::Call* call,
                         tint::wgsl::BuiltinFn builtin_fn) -> std::optional<TextureUsageType> {
            if (builtin_fn == wgsl::BuiltinFn::kTextureLoad) {
                if (call->Arguments()[0]
                        ->Type()
                        ->IsAnyOf<core::type::DepthTexture,
                                  core::type::DepthMultisampledTexture>()) {
                    return TextureUsageType::kTextureLoad;
                }
            }
            return {};
        };
        entry_point.has_texture_load_with_depth_texture =
            !GetTextureUsagesForEntryPoint(*func, filter).empty();
    }

    {
        auto filter = [](const tint::sem::Call* call,
                         tint::wgsl::BuiltinFn builtin_fn) -> std::optional<TextureUsageType> {
            if (IsTextureBuiltinThatUsesNonComparisonSampler(builtin_fn)) {
                if (call->Arguments()[0]
                        ->Type()
                        ->IsAnyOf<core::type::DepthTexture,
                                  core::type::DepthMultisampledTexture>()) {
                    return TextureUsageType::kDepthTextureWithNonComparisonSampler;
                }
            }
            return {};
        };
        entry_point.has_depth_texture_with_non_comparison_sampler =
            !GetTextureUsagesForEntryPoint(*func, filter).empty();
    }

    return entry_point;
}

EntryPoint Inspector::GetEntryPoint(const std::string& entry_point_name) {
    auto* func = FindEntryPointByName(entry_point_name);
    if (!func) {
        return EntryPoint();
    }
    return GetEntryPoint(func);
}

std::vector<EntryPoint> Inspector::GetEntryPoints() {
    std::vector<EntryPoint> result;

    for (auto* func : program_.AST().Functions()) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        result.push_back(GetEntryPoint(func));
    }

    return result;
}

std::map<OverrideId, Scalar> Inspector::GetOverrideDefaultValues() {
    std::map<OverrideId, Scalar> result;
    for (auto* var : program_.AST().GlobalVariables()) {
        auto* global = program_.Sem().Get<sem::GlobalVariable>(var);
        if (!global || !global->Declaration()->Is<ast::Override>()) {
            continue;
        }

        // If there are conflicting defintions for an override id, that is invalid
        // WGSL, so the resolver should catch it. Thus here the inspector just
        // assumes all definitions of the override id are the same, so only needs
        // to find the first reference to override id.
        auto override_id = global->Attributes().override_id.value();
        if (result.find(override_id) != result.end()) {
            continue;
        }

        if (global->Initializer()) {
            if (auto* value = global->Initializer()->ConstantValue()) {
                result[override_id] = Switch(
                    value->Type(),  //
                    [&](const core::type::I32*) { return Scalar(value->ValueAs<i32>()); },
                    [&](const core::type::U32*) { return Scalar(value->ValueAs<u32>()); },
                    [&](const core::type::F32*) { return Scalar(value->ValueAs<f32>()); },
                    [&](const core::type::F16*) {
                        // Default value of f16 override is also stored as float scalar.
                        return Scalar(static_cast<float>(value->ValueAs<f16>()));
                    },
                    [&](const core::type::Bool*) { return Scalar(value->ValueAs<bool>()); });
                continue;
            }
        }

        // No const-expression initializer for the override
        result[override_id] = Scalar();
    }

    return result;
}

std::map<std::string, OverrideId> Inspector::GetNamedOverrideIds() {
    std::map<std::string, OverrideId> result;
    for (auto* var : program_.AST().GlobalVariables()) {
        auto* global = program_.Sem().Get<sem::GlobalVariable>(var);
        if (auto override_id = global->Attributes().override_id) {
            auto name = var->name->symbol.Name();
            result[name] = override_id.value();
        }
    }
    return result;
}

std::vector<ResourceBinding> Inspector::GetResourceBindings(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    for (auto fn : {
             &Inspector::GetUniformBufferResourceBindings,
             &Inspector::GetStorageBufferResourceBindings,
             &Inspector::GetReadOnlyStorageBufferResourceBindings,
             &Inspector::GetSamplerResourceBindings,
             &Inspector::GetComparisonSamplerResourceBindings,
             &Inspector::GetSampledTextureResourceBindings,
             &Inspector::GetMultisampledTextureResourceBindings,
             &Inspector::GetStorageTextureResourceBindings,
             &Inspector::GetDepthTextureResourceBindings,
             &Inspector::GetDepthMultisampledTextureResourceBindings,
             &Inspector::GetExternalTextureResourceBindings,
             &Inspector::GetInputAttachmentResourceBindings,
         }) {
        AppendResourceBindings(&result, (this->*fn)(entry_point));
    }
    return result;
}

std::vector<ResourceBinding> Inspector::GetUniformBufferResourceBindings(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;

    auto* func_sem = program_.Sem().Get(func);
    for (auto& ruv : func_sem->TransitivelyReferencedUniformVariables()) {
        auto* var = ruv.first;
        auto binding_info = ruv.second;

        auto* unwrapped_type = var->Type()->UnwrapRef();

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kUniformBuffer;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.size = unwrapped_type->Size();
        entry.size_no_padding = entry.size;
        if (auto* str = unwrapped_type->As<sem::Struct>()) {
            entry.size_no_padding = str->SizeNoPadding();
        } else {
            entry.size_no_padding = entry.size;
        }
        entry.variable_name = var->Declaration()->name->symbol.Name();

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindings(
    const std::string& entry_point) {
    return GetStorageBufferResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding> Inspector::GetReadOnlyStorageBufferResourceBindings(
    const std::string& entry_point) {
    return GetStorageBufferResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding> Inspector::GetSamplerResourceBindings(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;

    auto* func_sem = program_.Sem().Get(func);
    for (auto& rs : func_sem->TransitivelyReferencedSamplerVariables()) {
        auto binding_info = rs.second;

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kSampler;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.variable_name = rs.first->Declaration()->name->symbol.Name();

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetComparisonSamplerResourceBindings(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;

    auto* func_sem = program_.Sem().Get(func);
    for (auto& rcs : func_sem->TransitivelyReferencedComparisonSamplerVariables()) {
        auto binding_info = rcs.second;

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kComparisonSampler;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.variable_name = rcs.first->Declaration()->name->symbol.Name();

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetSampledTextureResourceBindings(
    const std::string& entry_point) {
    return GetSampledTextureResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding> Inspector::GetMultisampledTextureResourceBindings(
    const std::string& entry_point) {
    return GetSampledTextureResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding> Inspector::GetStorageTextureResourceBindings(
    const std::string& entry_point) {
    return GetStorageTextureResourceBindingsImpl(entry_point);
}

std::vector<ResourceBinding> Inspector::GetTextureResourceBindings(
    const std::string& entry_point,
    const tint::TypeInfo* texture_type,
    ResourceBinding::ResourceType resource_type) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    auto* func_sem = program_.Sem().Get(func);
    for (auto& ref : func_sem->TransitivelyReferencedVariablesOfType(texture_type)) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        ResourceBinding entry;
        entry.resource_type = resource_type;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.variable_name = var->Declaration()->name->symbol.Name();

        auto* tex = var->Type()->UnwrapRef()->As<core::type::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetDepthTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &tint::TypeInfo::Of<core::type::DepthTexture>(),
                                      ResourceBinding::ResourceType::kDepthTexture);
}

std::vector<ResourceBinding> Inspector::GetDepthMultisampledTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point,
                                      &tint::TypeInfo::Of<core::type::DepthMultisampledTexture>(),
                                      ResourceBinding::ResourceType::kDepthMultisampledTexture);
}

std::vector<ResourceBinding> Inspector::GetExternalTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point,
                                      &tint::TypeInfo::Of<core::type::ExternalTexture>(),
                                      ResourceBinding::ResourceType::kExternalTexture);
}

std::vector<ResourceBinding> Inspector::GetInputAttachmentResourceBindings(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    auto* func_sem = program_.Sem().Get(func);
    for (auto& ref : func_sem->TransitivelyReferencedVariablesOfType(
             &tint::TypeInfo::Of<core::type::InputAttachment>())) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kInputAttachment;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

        auto* sem_var = var->As<sem::GlobalVariable>();
        TINT_ASSERT(sem_var);
        TINT_ASSERT(sem_var->Attributes().input_attachment_index);
        entry.input_attachmnt_index = sem_var->Attributes().input_attachment_index.value();

        auto* input_attachment_type = var->Type()->UnwrapRef()->As<core::type::InputAttachment>();
        auto* base_type = input_attachment_type->Type();
        entry.sampled_kind = BaseTypeToSampledKind(base_type);

        entry.variable_name = var->Declaration()->name->symbol.Name();

        entry.dim =
            TypeTextureDimensionToResourceBindingTextureDimension(input_attachment_type->Dim());

        result.push_back(entry);
    }

    return result;
}

VectorRef<SamplerTexturePair> Inspector::GetSamplerTextureUses(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    GenerateSamplerTargets();

    auto it = sampler_targets_->find(entry_point);
    if (it == sampler_targets_->end()) {
        return {};
    }
    return it->second;
}

void Inspector::GenerateSamplerTargets() {
    // Do not re-generate, since |program_| should not change during the lifetime
    // of the inspector.
    if (sampler_targets_ != nullptr) {
        return;
    }

    sampler_targets_ =
        std::make_unique<std::unordered_map<std::string, UniqueVector<SamplerTexturePair, 4>>>();

    auto& sem = program_.Sem();

    for (auto* node : program_.ASTNodes().Objects()) {
        auto* c = node->As<ast::CallExpression>();
        if (!c) {
            continue;
        }

        auto* call = sem.Get(c)->UnwrapMaterialize()->As<sem::Call>();
        if (!call) {
            continue;
        }

        auto* i = call->Target()->As<sem::BuiltinFn>();
        if (!i) {
            continue;
        }

        const auto& signature = i->Signature();
        int sampler_index = signature.IndexOf(core::ParameterUsage::kSampler);
        if (sampler_index == -1) {
            continue;
        }

        int texture_index = signature.IndexOf(core::ParameterUsage::kTexture);
        if (texture_index == -1) {
            continue;
        }

        auto* t = c->args[static_cast<size_t>(texture_index)];
        auto* s = c->args[static_cast<size_t>(sampler_index)];

        GetOriginatingResources(
            std::array<const ast::Expression*, 2>{t, s}, c,
            [&](std::array<const sem::GlobalVariable*, 2> globals, const sem::Function* fn) {
                Vector<const sem::Function*, 4> entry_points;
                if (fn->Declaration()->IsEntryPoint()) {
                    entry_points = {fn};
                } else {
                    entry_points = fn->AncestorEntryPoints();
                }

                auto texture_binding_point = *globals[0]->Attributes().binding_point;
                auto sampler_binding_point = *globals[1]->Attributes().binding_point;

                for (auto* entry_point : entry_points) {
                    const auto& ep_name = entry_point->Declaration()->name->symbol.Name();
                    (*sampler_targets_)[ep_name].Add(
                        {sampler_binding_point, texture_binding_point});
                }
            });
    }
}

template <size_t N, typename F>
void Inspector::GetOriginatingResources(std::array<const ast::Expression*, N> exprs,
                                        const ast::CallExpression* callsite,
                                        F&& callback) {
    if (DAWN_UNLIKELY(!program_.IsValid())) {
        TINT_ICE() << "attempting to get originating resources in invalid program";
        return;
    }

    auto& sem = program_.Sem();

    std::array<const sem::GlobalVariable*, N> globals{};
    std::array<const sem::Parameter*, N> parameters{};
    UniqueVector<const ast::CallExpression*, 8> callsites;

    for (size_t i = 0; i < N; i++) {
        const sem::Variable* root_ident = sem.GetVal(exprs[i])->RootIdentifier();
        if (auto* global = root_ident->As<sem::GlobalVariable>()) {
            globals[i] = global;
        } else if (auto* param = root_ident->As<sem::Parameter>()) {
            auto* func = tint::As<sem::Function>(param->Owner());
            if (func->CallSites().IsEmpty()) {
                // One or more of the expressions is a parameter, but this function
                // is not called. Ignore.
                return;
            }
            for (auto* call : func->CallSites()) {
                callsites.Add(call->Declaration());
            }
            parameters[i] = param;
        } else {
            TINT_ICE() << "cannot resolve originating resource with expression type "
                       << exprs[i]->TypeInfo().name;
            return;
        }
    }

    if (!callsites.IsEmpty()) {
        for (auto* call_expr : callsites) {
            // Make a copy of the expressions for this callsite
            std::array<const ast::Expression*, N> call_exprs = exprs;
            // Patch all the parameter expressions with their argument
            for (size_t i = 0; i < N; i++) {
                if (auto* param = parameters[i]) {
                    call_exprs[i] = call_expr->args[param->Index()];
                }
            }
            // Now call GetOriginatingResources() with from the callsite
            GetOriginatingResources(call_exprs, call_expr, callback);
        }
    } else {
        // All the expressions resolved to globals
        callback(globals, sem.Get(callsite)->Stmt()->Function());
    }
}

std::vector<SamplerTexturePair> Inspector::GetSamplerTextureUses(const std::string& entry_point,
                                                                 const BindingPoint& placeholder) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }
    auto* func_sem = program_.Sem().Get(func);

    std::vector<SamplerTexturePair> new_pairs;
    for (auto pair : func_sem->TextureSamplerPairs()) {
        auto* texture = pair.first->As<sem::GlobalVariable>();
        auto* sampler = pair.second ? pair.second->As<sem::GlobalVariable>() : nullptr;
        SamplerTexturePair new_pair;
        new_pair.sampler_binding_point =
            sampler ? *sampler->Attributes().binding_point : placeholder;
        new_pair.texture_binding_point = *texture->Attributes().binding_point;
        new_pairs.push_back(new_pair);
    }
    return new_pairs;
}

std::vector<std::string> Inspector::GetUsedExtensionNames() {
    auto& extensions = program_.Sem().Module()->Extensions();
    std::vector<std::string> out;
    out.reserve(extensions.Length());
    for (auto ext : extensions) {
        out.push_back(tint::ToString(ext));
    }
    return out;
}

std::vector<std::pair<std::string, Source>> Inspector::GetEnableDirectives() {
    std::vector<std::pair<std::string, Source>> result;

    // Ast nodes for enable directive are stored within global declarations list
    auto global_decls = program_.AST().GlobalDeclarations();
    for (auto* node : global_decls) {
        if (auto* enable = node->As<ast::Enable>()) {
            for (auto* ext : enable->extensions) {
                result.push_back({tint::ToString(ext->name), ext->source});
            }
        }
    }

    return result;
}

const ast::Function* Inspector::FindEntryPointByName(const std::string& name) {
    auto* func = program_.AST().Functions().Find(program_.Symbols().Get(name));
    if (!func) {
        diagnostics_.AddError(Source{}) << name << " was not found!";
        return nullptr;
    }

    if (!func->IsEntryPoint()) {
        diagnostics_.AddError(Source{}) << name << " is not an entry point!";
        return nullptr;
    }

    return func;
}

void Inspector::AddEntryPointInOutVariables(std::string name,
                                            std::string variable_name,
                                            const core::type::Type* type,
                                            VectorRef<const ast::Attribute*> attributes,
                                            std::optional<uint32_t> location,
                                            std::optional<uint32_t> color,
                                            std::optional<uint32_t> blend_src,
                                            std::vector<StageVariable>& variables) const {
    // Skip builtins.
    if (ast::HasAttribute<ast::BuiltinAttribute>(attributes)) {
        return;
    }

    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        // Recurse into members.
        for (auto* member : struct_ty->Members()) {
            AddEntryPointInOutVariables(name + "." + member->Name().Name(), member->Name().Name(),
                                        member->Type(), member->Declaration()->attributes,
                                        member->Attributes().location, member->Attributes().color,
                                        member->Attributes().blend_src, variables);
        }
        return;
    }

    // Base case: add the variable.

    StageVariable stage_variable;
    stage_variable.name = name;
    stage_variable.variable_name = variable_name;
    std::tie(stage_variable.component_type, stage_variable.composition_type) =
        CalculateComponentAndComposition(type);

    stage_variable.attributes.blend_src = blend_src;
    stage_variable.attributes.location = location;
    stage_variable.attributes.color = color;

    std::tie(stage_variable.interpolation_type, stage_variable.interpolation_sampling) =
        CalculateInterpolationData(attributes);

    variables.push_back(stage_variable);
}

bool Inspector::ContainsBuiltin(core::BuiltinValue builtin,
                                const core::type::Type* type,
                                VectorRef<const ast::Attribute*> attributes) const {
    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        // Recurse into members.
        for (auto* member : struct_ty->Members()) {
            if (ContainsBuiltin(builtin, member->Type(), member->Declaration()->attributes)) {
                return true;
            }
        }
        return false;
    }

    // Base case: check for builtin
    auto* builtin_declaration = ast::GetAttribute<ast::BuiltinAttribute>(attributes);
    if (!builtin_declaration) {
        return false;
    }
    return builtin_declaration->builtin == builtin;
}

std::optional<uint32_t> Inspector::GetClipDistancesBuiltinSize(const core::type::Type* type) const {
    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        for (auto* member : struct_ty->Members()) {
            if (ContainsBuiltin(core::BuiltinValue::kClipDistances, member->Type(),
                                member->Declaration()->attributes)) {
                auto* array_type = member->Type()->As<core::type::Array>();
                if (DAWN_UNLIKELY(array_type == nullptr)) {
                    TINT_ICE() << "clip_distances is not an array";
                }
                return array_type->ConstantCount();
            }
        }
    }

    return std::nullopt;
}

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindingsImpl(
    const std::string& entry_point,
    bool read_only) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    auto* func_sem = program_.Sem().Get(func);
    std::vector<ResourceBinding> result;
    for (auto& rsv : func_sem->TransitivelyReferencedStorageBufferVariables()) {
        auto* var = rsv.first;
        auto binding_info = rsv.second;

        if (read_only != (var->Access() == core::Access::kRead)) {
            continue;
        }

        auto* unwrapped_type = var->Type()->UnwrapRef();

        ResourceBinding entry;
        entry.resource_type = read_only ? ResourceBinding::ResourceType::kReadOnlyStorageBuffer
                                        : ResourceBinding::ResourceType::kStorageBuffer;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.size = unwrapped_type->Size();
        if (auto* str = unwrapped_type->As<sem::Struct>()) {
            entry.size_no_padding = str->SizeNoPadding();
        } else {
            entry.size_no_padding = entry.size;
        }
        entry.variable_name = var->Declaration()->name->symbol.Name();

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetSampledTextureResourceBindingsImpl(
    const std::string& entry_point,
    bool multisampled_only) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    auto* func_sem = program_.Sem().Get(func);
    auto referenced_variables = multisampled_only
                                    ? func_sem->TransitivelyReferencedMultisampledTextureVariables()
                                    : func_sem->TransitivelyReferencedSampledTextureVariables();
    for (auto& ref : referenced_variables) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        ResourceBinding entry;
        entry.resource_type = multisampled_only
                                  ? ResourceBinding::ResourceType::kMultisampledTexture
                                  : ResourceBinding::ResourceType::kSampledTexture;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.variable_name = var->Declaration()->name->symbol.Name();

        auto* texture_type = var->Type()->UnwrapRef()->As<core::type::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(texture_type->Dim());

        const core::type::Type* base_type = nullptr;
        if (multisampled_only) {
            base_type = texture_type->As<core::type::MultisampledTexture>()->Type();
        } else {
            base_type = texture_type->As<core::type::SampledTexture>()->Type();
        }
        entry.sampled_kind = BaseTypeToSampledKind(base_type);

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetStorageTextureResourceBindingsImpl(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    auto* func_sem = program_.Sem().Get(func);
    std::vector<ResourceBinding> result;
    for (auto& ref :
         func_sem->TransitivelyReferencedVariablesOfType<core::type::StorageTexture>()) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        auto* texture_type = var->Type()->UnwrapRef()->As<core::type::StorageTexture>();

        ResourceBinding entry;
        switch (texture_type->Access()) {
            case core::Access::kWrite:
                entry.resource_type = ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
                break;
            case core::Access::kReadWrite:
                entry.resource_type = ResourceBinding::ResourceType::kReadWriteStorageTexture;
                break;
            case core::Access::kRead:
                entry.resource_type = ResourceBinding::ResourceType::kReadOnlyStorageTexture;
                break;
            case core::Access::kUndefined:
                TINT_UNREACHABLE() << "unhandled storage texture access";
        }
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.variable_name = var->Declaration()->name->symbol.Name();

        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(texture_type->Dim());

        auto* base_type = texture_type->Type();
        entry.sampled_kind = BaseTypeToSampledKind(base_type);
        entry.image_format =
            TypeTexelFormatToResourceBindingTexelFormat(texture_type->TexelFormat());

        result.push_back(entry);
    }

    return result;
}

std::tuple<InterpolationType, InterpolationSampling> Inspector::CalculateInterpolationData(
    VectorRef<const ast::Attribute*> attributes) const {
    auto* interpolation_attribute = ast::GetAttribute<ast::InterpolateAttribute>(attributes);

    if (!interpolation_attribute) {
        return {InterpolationType::kPerspective, InterpolationSampling::kCenter};
    }

    auto ast_interpolation_type = interpolation_attribute->interpolation.type;
    auto ast_sampling_type = interpolation_attribute->interpolation.sampling;

    if (ast_sampling_type == core::InterpolationSampling::kUndefined) {
        if (ast_interpolation_type == core::InterpolationType::kFlat) {
            ast_sampling_type = core::InterpolationSampling::kFirst;
        } else {
            ast_sampling_type = core::InterpolationSampling::kCenter;
        }
    }

    auto interpolation_type = InterpolationType::kUnknown;
    switch (ast_interpolation_type) {
        case core::InterpolationType::kPerspective:
            interpolation_type = InterpolationType::kPerspective;
            break;
        case core::InterpolationType::kLinear:
            interpolation_type = InterpolationType::kLinear;
            break;
        case core::InterpolationType::kFlat:
            interpolation_type = InterpolationType::kFlat;
            break;
        case core::InterpolationType::kUndefined:
            break;
    }

    auto sampling_type = InterpolationSampling::kUnknown;
    switch (ast_sampling_type) {
        case core::InterpolationSampling::kUndefined:
            sampling_type = InterpolationSampling::kNone;
            break;
        case core::InterpolationSampling::kCenter:
            sampling_type = InterpolationSampling::kCenter;
            break;
        case core::InterpolationSampling::kCentroid:
            sampling_type = InterpolationSampling::kCentroid;
            break;
        case core::InterpolationSampling::kSample:
            sampling_type = InterpolationSampling::kSample;
            break;
        case core::InterpolationSampling::kFirst:
            sampling_type = InterpolationSampling::kFirst;
            break;
        case core::InterpolationSampling::kEither:
            sampling_type = InterpolationSampling::kEither;
            break;
    }

    return {interpolation_type, sampling_type};
}

uint32_t Inspector::ComputeWorkgroupStorageSize(const ast::Function* func) const {
    uint32_t total_size = 0;
    auto* func_sem = program_.Sem().Get(func);
    for (const sem::Variable* var : func_sem->TransitivelyReferencedGlobals()) {
        if (var->AddressSpace() == core::AddressSpace::kWorkgroup) {
            auto* ty = var->Type()->UnwrapRef();
            uint32_t align = ty->Align();
            uint32_t size = ty->Size();

            // This essentially matches std430 layout rules from GLSL, which are in
            // turn specified as an upper bound for Vulkan layout sizing. Since D3D
            // and Metal are even less specific, we assume Vulkan behavior as a
            // good-enough approximation everywhere.
            total_size += tint::RoundUp(16u, tint::RoundUp(align, size));
        }
    }

    return total_size;
}

uint32_t Inspector::ComputePushConstantSize(const ast::Function* func) const {
    uint32_t size = 0;
    auto* func_sem = program_.Sem().Get(func);
    for (const sem::Variable* var : func_sem->TransitivelyReferencedGlobals()) {
        if (var->AddressSpace() == core::AddressSpace::kPushConstant) {
            size += var->Type()->UnwrapRef()->Size();
        }
    }

    return size;
}

std::vector<PixelLocalMemberType> Inspector::ComputePixelLocalMemberTypes(
    const ast::Function* func) const {
    auto* func_sem = program_.Sem().Get(func);
    for (const sem::Variable* var : func_sem->TransitivelyReferencedGlobals()) {
        if (var->AddressSpace() != core::AddressSpace::kPixelLocal) {
            continue;
        }

        auto* str = var->Type()->UnwrapRef()->As<sem::Struct>();

        std::vector<PixelLocalMemberType> types;
        types.reserve(str->Members().Length());
        for (auto* member : str->Members()) {
            PixelLocalMemberType type = Switch(
                member->Type(),  //
                [&](const core::type::F32*) { return PixelLocalMemberType::kF32; },
                [&](const core::type::I32*) { return PixelLocalMemberType::kI32; },
                [&](const core::type::U32*) { return PixelLocalMemberType::kU32; },  //
                TINT_ICE_ON_NO_MATCH);
            types.push_back(type);
        }

        return types;
    }

    return {};
}

std::vector<Inspector::LevelSampleInfo> Inspector::GetTextureQueries(const std::string& ep_name) {
    const auto* ep = FindEntryPointByName(ep_name);
    if (!ep) {
        return {};
    }

    auto filter = [&](const tint::sem::Call* call,
                      tint::wgsl::BuiltinFn builtin_fn) -> std::optional<TextureUsageType> {
        switch (builtin_fn) {
            case wgsl::BuiltinFn::kTextureNumLevels: {
                return TextureUsageType::kTextureNumLevels;
            }
            case wgsl::BuiltinFn::kTextureDimensions: {
                if (call->Declaration()->args.Length() <= 1) {
                    // When textureDimension only takes a texture as the input,
                    // it doesn't require calls to textureNumLevels to clamp mip levels.
                    return {};
                }
                return TextureUsageType::kTextureNumLevels;
            }
            case wgsl::BuiltinFn::kTextureLoad: {
                if (call->Arguments()[0]
                        ->Type()
                        ->IsAnyOf<core::type::MultisampledTexture,
                                  core::type::DepthMultisampledTexture>()) {
                    // When textureLoad takes a multisampled texture as the input,
                    // it doesn't require to query the mip level.
                    return {};
                }
                return TextureUsageType::kTextureNumLevels;
            }
            case wgsl::BuiltinFn::kTextureNumSamples: {
                return TextureUsageType::kTextureNumSamples;
            }
            default:
                return {};
        }
    };

    auto usages = GetTextureUsagesForEntryPoint(*ep, filter);

    auto t = [](const TextureUsageInfo& info) -> LevelSampleInfo {
        return {
            info.type == TextureUsageType::kTextureNumSamples ? TextureQueryType::kTextureNumSamples
                                                              : TextureQueryType::kTextureNumLevels,
            info.group,
            info.binding,
        };
    };

    std::vector<LevelSampleInfo> res;
    std::transform(usages.begin(), usages.end(), std::back_inserter(res), t);
    return res;
}

std::vector<Inspector::TextureUsageInfo> Inspector::GetTextureUsagesForEntryPoint(
    const tint::ast::Function& ep,
    std::function<std::optional<TextureUsageType>(const tint::sem::Call* call,
                                                  tint::wgsl::BuiltinFn builtin_fn)> filter) {
    TINT_ASSERT(ep.IsEntryPoint());

    std::vector<TextureUsageInfo> res;

    std::unordered_set<BindingPoint> seen = {};

    Hashmap<const sem::Function*, Hashmap<const ast::Parameter*, TextureUsageType, 4>, 8>
        fn_to_data;

    auto record_function_param = [&fn_to_data](const sem::Function* func,
                                               const ast::Parameter* param, TextureUsageType type) {
        fn_to_data.GetOrAddZero(func).Add(param, type);
    };

    auto save_if_needed = [&res, &seen](const sem::GlobalVariable* global, TextureUsageType type) {
        auto binding = global->Attributes().binding_point.value();
        if (seen.insert(binding).second) {
            res.emplace_back(TextureUsageInfo{type, binding.group, binding.binding});
        }
    };

    auto& sem = program_.Sem();

    // This works in dependency order such that we'll see the texture call first and can record
    // any function parameter information and then as we walk up the function chain we can look
    // the call data.
    for (auto* fn_decl : sem.Module()->DependencyOrderedDeclarations()) {
        auto* fn = sem.Get<sem::Function>(fn_decl);
        if (!fn) {
            continue;
        }

        // This is an entrypoint, make sure it's the requested entry point
        if (fn->Declaration()->IsEntryPoint()) {
            if (fn->Declaration() != &ep) {
                continue;
            }
        } else {
            // Not an entry point, make sure it was called from the requested entry point
            if (!fn->HasAncestorEntryPoint(ep.name->symbol)) {
                continue;
            }
        }

        auto queryTextureBuiltin = [&](TextureUsageType type, const sem::Call* builtin_call,
                                       const sem::Variable* texture_sem = nullptr) {
            TINT_ASSERT(builtin_call);
            if (!texture_sem) {
                auto* texture_expr = builtin_call->Declaration()->args[0];
                texture_sem = sem.GetVal(texture_expr)->RootIdentifier();
            }
            tint::Switch(
                texture_sem,  //
                [&](const sem::GlobalVariable* global) { save_if_needed(global, type); },
                [&](const sem::Parameter* param) {
                    record_function_param(fn, param->Declaration(), type);
                },
                TINT_ICE_ON_NO_MATCH);
        };

        for (auto* call : fn->DirectCalls()) {
            // Builtin function call, record the texture information. If the used texture maps
            // back up to a function parameter just store the type of the call and we'll track the
            // function callback up in the `sem::Function` branch.
            tint::Switch(
                call->Target(),
                [&](const sem::BuiltinFn* builtin) {
                    auto type = filter(call, builtin->Fn());
                    if (type) {
                        queryTextureBuiltin(*type, call);
                    }
                },
                [&](const sem::Function* func) {
                    // A function call, check to see if any params needed to be tracked back to a
                    // global texture.

                    auto param_to_type = fn_to_data.Get(func);
                    if (!param_to_type) {
                        return;
                    }
                    TINT_ASSERT(call->Arguments().Length() == func->Declaration()->params.Length());

                    for (size_t i = 0; i < call->Arguments().Length(); i++) {
                        auto param = func->Declaration()->params[i];

                        // Determine if this had a texture we cared about
                        auto type = param_to_type->Get(param);
                        if (!type) {
                            continue;
                        }

                        auto* arg = call->Arguments()[i];
                        auto* texture_sem = arg->RootIdentifier();

                        tint::Switch(
                            texture_sem,
                            [&](const sem::GlobalVariable* global) {
                                save_if_needed(global, *type);
                            },
                            [&](const sem::Parameter* p) {
                                record_function_param(fn, p->Declaration(), *type);
                            },
                            TINT_ICE_ON_NO_MATCH);
                    }
                });
        }
    }

    return res;
}

}  // namespace tint::inspector
