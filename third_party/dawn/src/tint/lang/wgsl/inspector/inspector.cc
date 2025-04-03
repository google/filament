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
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
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

ResourceBinding ConvertBufferToResourceBinding(const tint::sem::GlobalVariable* buffer) {
    ResourceBinding result;
    result.bind_group = buffer->Attributes().binding_point->group;
    result.binding = buffer->Attributes().binding_point->binding;
    result.variable_name = buffer->Declaration()->name->symbol.Name();

    auto* unwrapped_type = buffer->Type()->UnwrapRef();
    result.size = unwrapped_type->Size();
    result.size_no_padding = result.size;
    if (auto* str = unwrapped_type->As<sem::Struct>()) {
        result.size_no_padding = str->SizeNoPadding();
    }

    if (buffer->AddressSpace() == core::AddressSpace::kStorage) {
        if (buffer->Access() == core::Access::kReadWrite) {
            result.resource_type = ResourceBinding::ResourceType::kStorageBuffer;
        } else {
            TINT_ASSERT(buffer->Access() == core::Access::kRead);
            result.resource_type = ResourceBinding::ResourceType::kReadOnlyStorageBuffer;
        }
    } else {
        TINT_ASSERT(buffer->AddressSpace() == core::AddressSpace::kUniform);
        result.resource_type = ResourceBinding::ResourceType::kUniformBuffer;
    }

    return result;
}

ResourceBinding ConvertHandleToResourceBinding(const tint::sem::GlobalVariable* handle) {
    ResourceBinding result;
    result.bind_group = handle->Attributes().binding_point->group;
    result.binding = handle->Attributes().binding_point->binding;
    result.variable_name = handle->Declaration()->name->symbol.Name();

    const core::type::Type* handle_type = handle->Type()->UnwrapRef();
    Switch(
        handle_type,

        [&](const core::type::Sampler* sampler) {
            if (sampler->Kind() == core::type::SamplerKind::kSampler) {
                result.resource_type = ResourceBinding::ResourceType::kSampler;
            } else {
                TINT_ASSERT(sampler->Kind() == core::type::SamplerKind::kComparisonSampler);
                result.resource_type = ResourceBinding::ResourceType::kComparisonSampler;
            }
        },

        [&](const core::type::SampledTexture* tex) {
            result.resource_type = ResourceBinding::ResourceType::kSampledTexture;
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());
            result.sampled_kind = BaseTypeToSampledKind(tex->Type());
        },
        [&](const core::type::MultisampledTexture* tex) {
            result.resource_type = ResourceBinding::ResourceType::kMultisampledTexture;
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());
            result.sampled_kind = BaseTypeToSampledKind(tex->Type());
        },
        [&](const core::type::DepthTexture* tex) {
            result.resource_type = ResourceBinding::ResourceType::kDepthTexture;
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());
        },
        [&](const core::type::DepthMultisampledTexture* tex) {
            result.resource_type = ResourceBinding::ResourceType::kDepthMultisampledTexture;
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());
        },
        [&](const core::type::StorageTexture* tex) {
            switch (tex->Access()) {
                case core::Access::kWrite:
                    result.resource_type = ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
                    break;
                case core::Access::kReadWrite:
                    result.resource_type = ResourceBinding::ResourceType::kReadWriteStorageTexture;
                    break;
                case core::Access::kRead:
                    result.resource_type = ResourceBinding::ResourceType::kReadOnlyStorageTexture;
                    break;
                case core::Access::kUndefined:
                    TINT_UNREACHABLE() << "unhandled storage texture access";
            }
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->Dim());
            result.sampled_kind = BaseTypeToSampledKind(tex->Type());
            result.image_format = TypeTexelFormatToResourceBindingTexelFormat(tex->TexelFormat());
        },
        [&](const core::type::ExternalTexture*) {
            result.resource_type = ResourceBinding::ResourceType::kExternalTexture;
            result.dim = ResourceBinding::TextureDimension::k2d;
        },

        [&](const core::type::InputAttachment* attachment) {
            result.resource_type = ResourceBinding::ResourceType::kInputAttachment;
            result.input_attachment_index = handle->Attributes().input_attachment_index.value();
            result.sampled_kind = BaseTypeToSampledKind(attachment->Type());
            result.dim = TypeTextureDimensionToResourceBindingTextureDimension(attachment->Dim());
        },

        TINT_ICE_ON_NO_MATCH);

    return result;
}

inspector::Override MkOverride(const sem::GlobalVariable* global, OverrideId id) {
    Override override;
    override.name = global->Declaration()->name->symbol.Name();
    override.id = id;

    auto* type = global->Type();
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
    return override;
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

            entry_point.uses_subgroup_matrix = UsesSubgroupMatrix(sem);

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
        auto* global = var->As<sem::GlobalVariable>();
        if (auto override_id = global->Attributes().override_id) {
            entry_point.overrides.push_back(MkOverride(global, override_id.value()));
        }
    }

    const auto& texture_metadata = ComputeTextureMetadata(entry_point.name);
    entry_point.has_texture_load_with_depth_texture =
        texture_metadata.has_texture_load_with_depth_texture;
    entry_point.has_depth_texture_with_non_comparison_sampler =
        texture_metadata.has_depth_texture_with_non_comparison_sampler;

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

        // If there are conflicting definitions for an override id, that is invalid
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
    auto* func_sem = program_.Sem().Get(func);
    for (auto& global : func_sem->TransitivelyReferencedGlobals()) {
        switch (global->AddressSpace()) {
            // Resources cannot be in these address spaces.
            case core::AddressSpace::kPrivate:
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kWorkgroup:
            case core::AddressSpace::kPushConstant:
            case core::AddressSpace::kPixelLocal:
            case core::AddressSpace::kIn:
            case core::AddressSpace::kOut:
            case core::AddressSpace::kUndefined:
                continue;

            case core::AddressSpace::kUniform:
            case core::AddressSpace::kStorage:
                result.push_back(ConvertBufferToResourceBinding(global));
                break;
            case core::AddressSpace::kHandle:
                result.push_back(ConvertHandleToResourceBinding(global));
                break;
        }
    }

    return result;
}

std::vector<SamplerTexturePair> Inspector::GetSamplerTextureUses(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    const auto& metadata = ComputeTextureMetadata(entry_point);
    std::vector<SamplerTexturePair> result = {metadata.sampling_pairs.begin(),
                                              metadata.sampling_pairs.end()};

    return result;
}

std::vector<SamplerTexturePair> Inspector::GetSamplerAndNonSamplerTextureUses(
    const std::string& entry_point,
    const BindingPoint& non_sampler_placeholder) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    // This function adds texture usages with a fake sampler binding point, only for builtins that
    // don't use a sampler. Others are returned as usual.
    std::vector<SamplerTexturePair> result = GetSamplerTextureUses(entry_point);

    const auto& metadata = ComputeTextureMetadata(entry_point);
    for (const auto& texture : metadata.textures_used_without_samplers) {
        SamplerTexturePair new_pair;
        new_pair.sampler_binding_point = non_sampler_placeholder;
        new_pair.texture_binding_point = texture;
        result.push_back(new_pair);
    }

    return result;
}

std::vector<Inspector::LevelSampleInfo> Inspector::GetTextureQueries(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    const auto& metadata = ComputeTextureMetadata(entry_point);

    std::vector<LevelSampleInfo> result;
    for (const auto& texture : metadata.textures_with_num_levels) {
        result.push_back({TextureQueryType::kTextureNumLevels, texture.group, texture.binding});
    }
    for (const auto& texture : metadata.textures_with_num_samples) {
        result.push_back({TextureQueryType::kTextureNumSamples, texture.group, texture.binding});
    }

    return result;
}

const Inspector::EntryPointTextureMetadata& Inspector::ComputeTextureMetadata(
    const std::string& entry_point) {
    auto [entry, inserted] = texture_metadata_.emplace(entry_point, EntryPointTextureMetadata{});
    auto& metadata = entry->second;
    if (!inserted) {
        return metadata;
    }

    Symbol entry_point_symbol = program_.Symbols().Get(entry_point);
    auto& sem = program_.Sem();

    using GlobalSet = UniqueVector<const sem::GlobalVariable*, 4>;

    // Stores for each function parameter which globals are statically determined to be used as an
    // argument to the function call.
    Hashmap<const sem::Function*, Hashmap<const sem::Parameter*, GlobalSet, 2>, 8>
        globals_for_handle_parameters;
    auto AddGlobalsAsParameter = [&](const sem::Function* fn, const sem::Parameter* param,
                                     const GlobalSet* vars) {
        auto& globals = globals_for_handle_parameters.GetOrAddZero(fn).GetOrAddZero(param);
        for (const auto* var : *vars) {
            globals.Add(var);
        }
    };

    // Returns the set of globals for a handle argument. A scratch set is passed in to be used when
    // the argument directly references a global so that a reference on the stack can be passed.
    // TODO(343500108): Use std::span when we have C++20
    auto GetGlobalsForArgument = [&](const sem::Function* fn, const sem::ValueExpression* argument,
                                     GlobalSet* scratch_global) -> const GlobalSet* {
        TINT_ASSERT(scratch_global->IsEmpty());

        // Handle parameter can only be identifiers.
        auto* identifier = argument->RootIdentifier();

        return tint::Switch(
            identifier,
            [&](const sem::GlobalVariable* global) {
                scratch_global->Add(global);
                return scratch_global;
            },
            [&](const sem::Parameter* parameter) {
                return &*(globals_for_handle_parameters.Get(fn)->Get(parameter));
            },
            TINT_ICE_ON_NO_MATCH);
    };

    // The actual logic to compute the relevant metadata when a builtin call is found. It gets the
    // set of statically determined globals for the texture and sampler arguments.
    auto RecordBuiltinCallMetadata = [&](const sem::Call* call, const sem::BuiltinFn* builtin,
                                         const GlobalSet& textures, const GlobalSet& samplers) {
        // All builtins with samplers also take a texture.
        TINT_ASSERT(!textures.IsEmpty());

        // Compute the statically used texture+sampler pairs.
        for (const auto* sampler : samplers) {
            auto sampler_binding_point = sampler->Attributes().binding_point.value();

            for (const auto* texture : textures) {
                auto texture_binding_point = texture->Attributes().binding_point.value();
                metadata.sampling_pairs.insert({sampler_binding_point, texture_binding_point});
            }
        }

        // Also gather uses of non-storage textures that are used without a sampler
        const auto* texture_type =
            call->Arguments()[static_cast<size_t>(
                                  builtin->Signature().IndexOf(core::ParameterUsage::kTexture))]
                ->Type();
        if (samplers.IsEmpty() && !texture_type->Is<core::type::StorageTexture>()) {
            for (const auto* texture : textures) {
                auto texture_binding_point = texture->Attributes().binding_point.value();
                metadata.textures_used_without_samplers.insert(texture_binding_point);
            }
        }

        bool uses_num_levels = false;
        switch (builtin->Fn()) {
            case wgsl::BuiltinFn::kTextureNumLevels:
                uses_num_levels = true;
                break;

            case wgsl::BuiltinFn::kTextureDimensions:
                // When textureDimension takes more than one argument, one of them is a mip level
                // that will get clamped using textureNumLevels.
                uses_num_levels = call->Arguments().Length() > 1;
                break;

            case wgsl::BuiltinFn::kTextureLoad:
                // textureLoad uses textureNumLevels to clamp the level, unless the texture type
                // doesn't support mipmapping.
                uses_num_levels = !texture_type->IsAnyOf<core::type::MultisampledTexture,
                                                         core::type::DepthMultisampledTexture>();
                metadata.has_texture_load_with_depth_texture |=
                    texture_type
                        ->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>();
                break;

            case wgsl::BuiltinFn::kTextureSample:
            case wgsl::BuiltinFn::kTextureSampleLevel:
            case wgsl::BuiltinFn::kTextureGather:
                metadata.has_depth_texture_with_non_comparison_sampler |=
                    texture_type
                        ->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>();
                break;

            case wgsl::BuiltinFn::kTextureNumSamples:
                for (const auto* texture : textures) {
                    auto texture_binding_point = texture->Attributes().binding_point.value();
                    metadata.textures_with_num_samples.insert(texture_binding_point);
                }
                break;

            default:
                break;
        }

        if (uses_num_levels) {
            for (const auto* texture : textures) {
                auto texture_binding_point = texture->Attributes().binding_point.value();
                metadata.textures_with_num_levels.insert(texture_binding_point);
            }
        }
    };

    // Iterate the call graph in reverse topological order such that function callers come before
    // their callee.
    auto declarations = sem.Module()->DependencyOrderedDeclarations();
    for (auto rit = declarations.rbegin(); rit != declarations.rend(); rit++) {
        auto* fn = sem.Get<sem::Function>(*rit);
        if (!fn || !fn->HasCallGraphEntryPoint(entry_point_symbol)) {
            continue;
        }

        for (auto* call : fn->DirectCalls()) {
            tint::Switch(
                call->Target(),

                // Propagate the used globals for handle parameters of function calls.
                [&](const sem::Function* callee) {
                    for (size_t i = 0; i < call->Arguments().Length(); i++) {
                        auto parameter = sem.Get(callee->Declaration()->params[i]);
                        if (!parameter->Type()->IsHandle()) {
                            continue;
                        }

                        // Handle parameter can only be identifiers.
                        GlobalSet scratch_global;
                        const auto* globals =
                            GetGlobalsForArgument(fn, call->Arguments()[i], &scratch_global);
                        AddGlobalsAsParameter(callee, parameter, globals);
                    }
                },

                [&](const sem::BuiltinFn* builtin) {
                    // Find sampler / texture parameters and skip over builtin calls without any.
                    const auto& signature = builtin->Signature();
                    int texture_index = signature.IndexOf(core::ParameterUsage::kTexture);
                    int sampler_index = signature.IndexOf(core::ParameterUsage::kSampler);

                    if (texture_index == -1) {
                        return;
                    }

                    // Compute the set of globals used for the texture/sampler parameter.
                    // It will either point to a GlobalSet on the stack when a global is used
                    // directly, or to the contents of globals_for_handle_parameters.
                    GlobalSet scratch_sampler_global;
                    const GlobalSet* sampler_globals = &scratch_sampler_global;
                    if (sampler_index != -1) {
                        sampler_globals = GetGlobalsForArgument(
                            fn, call->Arguments()[size_t(sampler_index)], &scratch_sampler_global);
                    }

                    GlobalSet scratch_texture_global;
                    const GlobalSet* texture_globals = GetGlobalsForArgument(
                        fn, call->Arguments()[size_t(texture_index)], &scratch_texture_global);

                    RecordBuiltinCallMetadata(call, builtin, *texture_globals, *sampler_globals);
                });
        }
    }

    return metadata;
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

bool Inspector::UsesSubgroupMatrix(const sem::Function* func) const {
    if (func->DirectlyUsedSubgroupMatrix()) {
        return true;
    }
    for (auto& call : func->TransitivelyCalledFunctions()) {
        if (call->DirectlyUsedSubgroupMatrix()) {
            return true;
        }
    }
    return false;
}

std::vector<Override> Inspector::Overrides() {
    std::vector<Override> results;

    for (auto* var : program_.AST().GlobalVariables()) {
        auto* global = program_.Sem().Get<sem::GlobalVariable>(var);
        if (!global || !global->Declaration()->Is<ast::Override>()) {
            continue;
        }

        results.push_back(MkOverride(global, global->Attributes().override_id.value()));
    }
    return results;
}

}  // namespace tint::inspector
