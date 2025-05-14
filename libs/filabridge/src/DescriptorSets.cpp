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
#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>
#include <utils/debug.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace filament::descriptor_sets {

using namespace backend;

// used for post-processing passes
static constexpr std::initializer_list<DescriptorSetLayoutBinding> postProcessDescriptorSetLayoutList = {
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
};

// used to generate shadow-maps
static constexpr std::initializer_list<DescriptorSetLayoutBinding> depthVariantDescriptorSetLayoutList = {
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
};

// ssrVariantDescriptorSetLayout must match perViewDescriptorSetLayout's vertex stage. This is
// because the SSR variant is always using the "standard" vertex shader (i.e. there is no
// dedicated SSR vertex shader), which uses perViewDescriptorSetLayout.
// This means that PerViewBindingPoints::SHADOWS must be in the layout even though it's not used
// by the SSR variant.
static constexpr std::initializer_list<DescriptorSetLayoutBinding> ssrVariantDescriptorSetLayoutList = {
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOWS        },
    { DescriptorType::SAMPLER_2D_FLOAT,                          ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::STRUCTURE      },
    { DescriptorType::SAMPLER_2D_ARRAY_FLOAT,                    ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSR            },
};

// Used for generating the color pass (i.e. the main pass). This is in fact a template that gets
// declined into 8 different layouts, based on variants.
//
// Note about the SHADOW_MAP binding points:
// This descriptor can either be a SAMPLER_FLOAT or a SAMPLER_DEPTH,
// and there are 3 cases to consider:
//
//          | TextureType | CompareMode | Filtered | SamplerType | Variant |
// ---------+-------------+-------------+----------+-------------+---------+
//  PCF     |    DEPTH    |    COMPARE  |   Yes    |    DEPTH    |    -    |
//  VSM     |    FLOAT    |     NONE    |   Yes    |    FLOAT    |   VSM   |
//  OTHER   |    DEPTH    |     NONE    |   No     |    FLOAT    |   VSM   |
//
// The SamplerType to use depends on the Variant. Variant::VSM is set for all cases except PCM.
//
static constexpr std::initializer_list<DescriptorSetLayoutBinding> perViewDescriptorSetLayoutList = {
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORMS },
    { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOWS        },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::LIGHTS         },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::RECORD_BUFFER  },
    { DescriptorType::UNIFORM_BUFFER,                            ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FROXEL_BUFFER  },
    { DescriptorType::SAMPLER_2D_FLOAT,                          ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::STRUCTURE      },
    { DescriptorType::SAMPLER_2D_ARRAY_DEPTH,                    ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SHADOW_MAP     },
    { DescriptorType::SAMPLER_2D_FLOAT,                          ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::IBL_DFG_LUT    },
    { DescriptorType::SAMPLER_CUBE_FLOAT,                        ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::IBL_SPECULAR   },
    { DescriptorType::SAMPLER_2D_ARRAY_FLOAT,                    ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSAO           },
    { DescriptorType::SAMPLER_2D_ARRAY_FLOAT,                    ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::SSR            },
    { DescriptorType::SAMPLER_CUBE_FLOAT,                        ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FOG            },
};

static constexpr std::initializer_list<DescriptorSetLayoutBinding> perRenderableDescriptorSetLayoutList = {
    { DescriptorType::UNIFORM_BUFFER,           ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::OBJECT_UNIFORMS, DescriptorFlags::DYNAMIC_OFFSET },
    { DescriptorType::UNIFORM_BUFFER,           ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::BONES_UNIFORMS,  DescriptorFlags::DYNAMIC_OFFSET },
    { DescriptorType::UNIFORM_BUFFER,           ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::MORPHING_UNIFORMS         },
    { DescriptorType::SAMPLER_2D_ARRAY_FLOAT,   ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::MORPH_TARGET_POSITIONS    },
    { DescriptorType::SAMPLER_2D_ARRAY_FLOAT,   ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::MORPH_TARGET_TANGENTS     },
    { DescriptorType::SAMPLER_2D_FLOAT,         ShaderStageFlags::VERTEX                             ,  +PerRenderableBindingPoints::BONES_INDICES_AND_WEIGHTS },
};

struct PairSamplerTypeFormatHasher {
    std::size_t operator()(const std::pair<SamplerType, SamplerFormat>& p) const {
        using UnderlyingSamplerType = std::underlying_type_t<SamplerType>;
        using UnderlyingSamplerFormat = std::underlying_type_t<SamplerFormat>;
        std::size_t seed = 0;
        std::size_t const hash1 = std::hash<UnderlyingSamplerType>{}(
            static_cast<UnderlyingSamplerType>(p.first)
        );
        std::size_t const hash2 = std::hash<UnderlyingSamplerFormat>{}(
            static_cast<UnderlyingSamplerFormat>(p.second)
        );
        seed ^= hash1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

static const std::unordered_map<
    std::pair<SamplerType, SamplerFormat>, DescriptorType, PairSamplerTypeFormatHasher> sDescriptorTypeMap{
        {{ SamplerType::SAMPLER_2D, SamplerFormat::INT }, DescriptorType::SAMPLER_2D_INT },
        {{ SamplerType::SAMPLER_2D, SamplerFormat::UINT }, DescriptorType::SAMPLER_2D_UINT },
        {{ SamplerType::SAMPLER_2D, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_2D_FLOAT },
        {{ SamplerType::SAMPLER_2D, SamplerFormat::SHADOW }, DescriptorType::SAMPLER_2D_DEPTH },
        {{ SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::INT }, DescriptorType::SAMPLER_2D_ARRAY_INT },
        {{ SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::UINT }, DescriptorType::SAMPLER_2D_ARRAY_UINT },
        {{ SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_2D_ARRAY_FLOAT },
        {{ SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::SHADOW }, DescriptorType::SAMPLER_2D_ARRAY_DEPTH },
        {{ SamplerType::SAMPLER_CUBEMAP, SamplerFormat::INT }, DescriptorType::SAMPLER_CUBE_INT },
        {{ SamplerType::SAMPLER_CUBEMAP, SamplerFormat::UINT }, DescriptorType::SAMPLER_CUBE_UINT },
        {{ SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_CUBE_FLOAT },
        {{ SamplerType::SAMPLER_CUBEMAP, SamplerFormat::SHADOW }, DescriptorType::SAMPLER_CUBE_DEPTH },
        {{ SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::INT }, DescriptorType::SAMPLER_CUBE_ARRAY_INT },
        {{ SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::UINT }, DescriptorType::SAMPLER_CUBE_ARRAY_UINT },
        {{ SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT },
        {{ SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::SHADOW }, DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH },
        {{ SamplerType::SAMPLER_3D, SamplerFormat::INT }, DescriptorType::SAMPLER_3D_INT },
        {{ SamplerType::SAMPLER_3D, SamplerFormat::UINT }, DescriptorType::SAMPLER_3D_UINT },
        {{ SamplerType::SAMPLER_3D, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_3D_FLOAT },
        {{ SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT }, DescriptorType::SAMPLER_EXTERNAL }
};

// used for post-processing passes
static DescriptorSetLayout const postProcessDescriptorSetLayout{ utils::StaticString("postProcess"),
    postProcessDescriptorSetLayoutList };

// used to generate shadow-maps
static DescriptorSetLayout const depthVariantDescriptorSetLayout{
    utils::StaticString("depthVariant"), depthVariantDescriptorSetLayoutList
};

static DescriptorSetLayout const ssrVariantDescriptorSetLayout{ utils::StaticString("ssrVariant"),
    ssrVariantDescriptorSetLayoutList };

// Used for generating the color pass (i.e. the main pass). This is in fact a template that gets
// declined into 8 different layouts, based on variants.
static DescriptorSetLayout perViewDescriptorSetLayout = { utils::StaticString("perView"),
    perViewDescriptorSetLayoutList };

static DescriptorSetLayout perRenderableDescriptorSetLayout = {
    utils::StaticString("perRenderable"), perRenderableDescriptorSetLayoutList
};

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

utils::CString getDescriptorName(DescriptorSetBindingPoints const set,
        descriptor_binding_t const binding) noexcept {
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
            auto const pos = set0.find(binding);
            assert_invariant(pos != set0.end());
            return { pos->second.data(), pos->second.size() };
        }
        case DescriptorSetBindingPoints::PER_RENDERABLE: {
            auto const pos = set1.find(binding);
            assert_invariant(pos != set1.end());
            return { pos->second.data(), pos->second.size() };
        }
        case DescriptorSetBindingPoints::PER_MATERIAL: {
            assert_invariant(binding < 1);
            return "MaterialParams";
        }
    }
    return "Unknown";
}

DescriptorSetLayout getPerViewDescriptorSetLayout(
        MaterialDomain const domain,
        bool const isLit, bool const isSSR, bool const hasFog,
        bool const isVSM) noexcept {

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
            if (!isSSR) {
                layout.bindings.erase(
                        std::remove_if(layout.bindings.begin(), layout.bindings.end(),
                                [](auto const& entry) {
                                    return entry.binding == PerViewBindingPoints::SSR;
                                }),
                        layout.bindings.end());

            }
            // remove fog descriptor if filtered out
            if (!hasFog) {
                layout.bindings.erase(
                        std::remove_if(layout.bindings.begin(), layout.bindings.end(),
                                [](auto const& entry) {
                                    return entry.binding == PerViewBindingPoints::FOG;
                                }),
                        layout.bindings.end());
            }

            // change the SHADOW_MAP descriptor type for VSM
            if (isVSM) {
                auto const pos = std::find_if(layout.bindings.begin(), layout.bindings.end(),
                        [](auto const& v) {
                            return v.binding == PerViewBindingPoints::SHADOW_MAP;
                        });
                if (pos != layout.bindings.end()) {
                    pos->type = DescriptorType::SAMPLER_2D_ARRAY_FLOAT;
                }
            }
            return layout;
        }
        case MaterialDomain::POST_PROCESS:
            return postProcessDescriptorSetLayout;
        case MaterialDomain::COMPUTE:
            // TODO: what's the layout for compute?
            return postProcessDescriptorSetLayout;
    }
}

DescriptorSetLayout getPerViewDescriptorSetLayoutWithVariant(
        Variant const variant,
        MaterialDomain const domain,
        bool const isLit, bool const isSSR, bool const hasFog) noexcept {
    if (Variant::isValidDepthVariant(variant)) {
        return depthVariantDescriptorSetLayout;
    }
    if (Variant::isSSRVariant(variant)) {
        return ssrVariantDescriptorSetLayout;
    }
    // We need to filter out all the descriptors not included in the "resolved" layout below
    return getPerViewDescriptorSetLayout(domain, isLit, isSSR, hasFog,
            Variant::isVSMVariant(variant));
}

DescriptorType getDescriptorType(SamplerType const type, SamplerFormat const format) {
    auto const pos = sDescriptorTypeMap.find({ type, format });
    FILAMENT_CHECK_PRECONDITION(pos != sDescriptorTypeMap.end())
            << "Incompatible Sampler Format " << to_string(format)
            << " and Type " << to_string(type);
    return pos->second;
}

template<class ITERATOR, class PREDICATE>
constexpr static ITERATOR find_if(ITERATOR first, ITERATOR last, PREDICATE pred) {
    for (; first != last; ++first)
        if (pred(*first)) break;
    return first;
}

constexpr static bool checkConsistency() noexcept {
    // check that all descriptors that apply to the vertex stage in perViewDescriptorSetLayout
    // are present in ssrVariantDescriptorSetLayout; meaning that the latter is compatible
    // with the former.
    for (auto const& r: perViewDescriptorSetLayoutList) {
        if (hasShaderType(r.stageFlags, ShaderStage::VERTEX)) {
            auto const pos = find_if(
                    ssrVariantDescriptorSetLayoutList.begin(),
                    ssrVariantDescriptorSetLayoutList.end(),
                    [r](auto const& l) {
                        return l.count == r.count &&
                               l.type == r.type &&
                               l.binding == r.binding &&
                               l.flags == r.flags &&
                               l.stageFlags == r.stageFlags;
                    });
            if (pos == ssrVariantDescriptorSetLayoutList.end()) {
                return false;
            }
        }
    }
    return true;
}

static_assert(checkConsistency(), "ssrVariantDescriptorSetLayout is not compatible with "
        "perViewDescriptorSetLayout");

} // namespace filament::descriptor_sets
