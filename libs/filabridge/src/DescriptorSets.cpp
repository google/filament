/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "private/filament/DescriptorSets.h"

#include <private/filament/EngineEnums.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/debug.h>

#include <algorithm>
#include <unordered_map>
#include <string_view>

namespace filament::descriptor_sets {

using namespace backend;

static DescriptorSetLayout const postProcessDescriptorSetLayout{{
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
}};

static DescriptorSetLayout const depthVariantDescriptorSetLayout{{
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
}};

// ssrVariantDescriptorSetLayout must match perViewDescriptorSetLayout's vertex stage. This is
// because the SSR variant is always using the "standard" vertex shader (i.e. there is no
// dedicated SSR vertex shader), which uses perViewDescriptorSetLayout.
// This means that PerViewBindingPoints::SHADOWS must be in the layout even though it's not used
// by the SSR variant.
static DescriptorSetLayout const ssrVariantDescriptorSetLayout{{
   { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
   { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOWS        },
   { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::STRUCTURE      },
   { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSR            },
}};

static DescriptorSetLayout perViewDescriptorSetLayout = {{
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOWS        },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::LIGHTS         },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::RECORD_BUFFER  },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FROXEL_BUFFER  },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::STRUCTURE      },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOW_MAP     },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::IBL_DFG_LUT    },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::IBL_SPECULAR   },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSAO           },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSR            },
    { DescriptorType::SAMPLER,                                   ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FOG            },
}};

static DescriptorSetLayout perRenderableDescriptorSetLayout = {{
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::OBJECT_UNIFORMS, DescriptorFlags::DYNAMIC_OFFSET },
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::BONES_UNIFORMS,  DescriptorFlags::DYNAMIC_OFFSET },
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::MORPHING_UNIFORMS         },
    { DescriptorType::SAMPLER,        ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::MORPH_TARGET_POSITIONS    },
    { DescriptorType::SAMPLER,        ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::MORPH_TARGET_TANGENTS     },
    { DescriptorType::SAMPLER,        ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::BONES_INDICES_AND_WEIGHTS },
}};

DescriptorSetLayout const& getPostProcessLayout() noexcept {
    return postProcessDescriptorSetLayout;
}

DescriptorSetLayout const& getDepthVariantLayout() noexcept {
    return depthVariantDescriptorSetLayout;
}

DescriptorSetLayout const& getSsrVariantLayout() noexcept {
    return ssrVariantDescriptorSetLayout;
}

DescriptorSetLayout const& getPerRenderableLayout() noexcept {
    return perRenderableDescriptorSetLayout;
}

utils::CString getDescriptorName(DescriptorSetBindingPoints set,
        descriptor_binding_t binding) noexcept {
    using namespace std::literals;

    static std::unordered_map<descriptor_binding_t, std::string_view> const set0{{
        { +PerViewBindingPoints::FRAME_UNIFORMS, "FrameUniforms"sv },
        { +PerViewBindingPoints::SHADOWS,        "ShadowUniforms"sv },
        { +PerViewBindingPoints::LIGHTS,         "LightsUniforms"sv },
        { +PerViewBindingPoints::RECORD_BUFFER,  "FroxelRecordUniforms"sv },
        { +PerViewBindingPoints::FROXEL_BUFFER,  "FroxelsUniforms"sv },
        { +PerViewBindingPoints::STRUCTURE,      "sampler0_structure"sv },
        { +PerViewBindingPoints::SHADOW_MAP,     "sampler0_shadowMap"sv },
        { +PerViewBindingPoints::IBL_DFG_LUT,    "sampler0_iblDFG"sv },
        { +PerViewBindingPoints::IBL_SPECULAR,   "sampler0_iblSpecular"sv },
        { +PerViewBindingPoints::SSAO,           "sampler0_ssao"sv },
        { +PerViewBindingPoints::SSR,            "sampler0_ssr"sv },
        { +PerViewBindingPoints::FOG,            "sampler0_fog"sv },
    }};

    static std::unordered_map<descriptor_binding_t, std::string_view> const set1{{
        { +PerRenderableBindingPoints::OBJECT_UNIFORMS,             "ObjectUniforms"sv },
        { +PerRenderableBindingPoints::BONES_UNIFORMS,              "BonesUniforms"sv },
        { +PerRenderableBindingPoints::MORPHING_UNIFORMS,           "MorphingUniforms"sv },
        { +PerRenderableBindingPoints::MORPH_TARGET_POSITIONS,      "sampler1_positions"sv },
        { +PerRenderableBindingPoints::MORPH_TARGET_TANGENTS,       "sampler1_tangents"sv },
        { +PerRenderableBindingPoints::BONES_INDICES_AND_WEIGHTS,   "sampler1_indicesAndWeights"sv },
    }};

    switch (set) {
        case DescriptorSetBindingPoints::PER_VIEW: {
            auto pos = set0.find(binding);
            assert_invariant(pos != set0.end());
            return { pos->second.data(), pos->second.size() };
        }
        case DescriptorSetBindingPoints::PER_RENDERABLE: {
            auto pos = set1.find(binding);
            assert_invariant(pos != set1.end());
            return { pos->second.data(), pos->second.size() };
        }
        case DescriptorSetBindingPoints::PER_MATERIAL: {
            assert_invariant(binding < 1);
            return "MaterialParams";
        }
    }
}

DescriptorSetLayout getPerViewDescriptorSetLayout(
        MaterialDomain domain,
        UserVariantFilterMask variantFilter,
        bool isLit,
        ReflectionMode reflectionMode,
        RefractionMode refractionMode) noexcept {

    bool const ssr = reflectionMode == ReflectionMode::SCREEN_SPACE ||
                     refractionMode == RefractionMode::SCREEN_SPACE;

    switch (domain) {
        case MaterialDomain::SURFACE: {
            //
            // CAVEAT: The logic here must match MaterialBuilder::checkMaterialLevelFeatures()
            //
            auto layout = perViewDescriptorSetLayout;
            // remove descriptors not needed for unlit materials
            if (!isLit) {
                layout.bindings.erase(
                        std::remove_if(layout.bindings.begin(), layout.bindings.end(),
                                [](auto const& entry) {
                                    return  entry.binding == PerViewBindingPoints::IBL_DFG_LUT ||
                                            entry.binding == PerViewBindingPoints::IBL_SPECULAR;
                                }),
                        layout.bindings.end());
            }
            // remove descriptors not needed for SSRs
            if (!ssr) {
                layout.bindings.erase(
                        std::remove_if(layout.bindings.begin(), layout.bindings.end(),
                                [](auto const& entry) {
                                    return entry.binding == PerViewBindingPoints::SSR;
                                }),
                        layout.bindings.end());

            }
            // remove fog descriptor if filtered out
            if (variantFilter & (UserVariantFilterMask)UserVariantFilterBit::FOG) {
                layout.bindings.erase(
                        std::remove_if(layout.bindings.begin(), layout.bindings.end(),
                                [](auto const& entry) {
                                    return entry.binding == PerViewBindingPoints::FOG;
                                }),
                        layout.bindings.end());
            }
            return layout;
        }
        case MaterialDomain::POST_PROCESS:
            return descriptor_sets::getPostProcessLayout();
        case MaterialDomain::COMPUTE:
            // TODO: what's the layout for compute?
            return descriptor_sets::getPostProcessLayout();
    }
}

} // namespace filament::descriptor_sets
